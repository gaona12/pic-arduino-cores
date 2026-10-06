/*
 * tinybld_load.c - Cargador del Tiny PIC Bootloader en C (macOS/Linux).
 *
 * Reemplaza al cargador en Python. Parte del ecosistema en C del core.
 * Flujo: abre el puerto serie (termios), hace el handshake 0xC1, lee el
 * .hex Intel, reubica el vector de reset como el bootloader espera y graba
 * por bloques con checksum.
 *
 * Uso:
 *   tinybld_load -p /dev/cu.usbserial-130 -f programa.hex
 *   tinybld_load -p /dev/cu.usbserial-130 --detect
 *
 * Protocolo basado en Tiny PIC Bootloader (Claudiu Chiculita) para 16F877A.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
#include <time.h>

#define MAXFLASH 0x2000      /* 8192 words (16F877A) */
#define BSIZE    100         /* tamanio del bootloader en words */
#define PIC_TYPE 0x31        /* byte de tipo 16F876A/877A */

static int ser_fd = -1;

static int ser_open(const char *port) {
    int fd = open(port, O_RDWR | O_NOCTTY | O_NONBLOCK);
    if (fd < 0) return -1;
    struct termios t;
    if (tcgetattr(fd, &t) != 0) { close(fd); return -1; }
    cfmakeraw(&t);
    cfsetispeed(&t, B115200);
    cfsetospeed(&t, B115200);
    t.c_cflag |= (CLOCAL | CREAD);
    t.c_cflag &= ~CRTSCTS;
    t.c_cc[VMIN] = 0;
    t.c_cc[VTIME] = 1;   /* 0.1s timeout */
    tcsetattr(fd, TCSANOW, &t);
    fcntl(fd, F_SETFL, 0);
    return fd;
}

static int ser_read_byte(unsigned char *b, int timeout_ms) {
    struct timespec start, now;
    clock_gettime(CLOCK_MONOTONIC, &start);
    for (;;) {
        int n = read(ser_fd, b, 1);
        if (n == 1) return 1;
        clock_gettime(CLOCK_MONOTONIC, &now);
        long ms = (now.tv_sec - start.tv_sec) * 1000 +
                  (now.tv_nsec - start.tv_nsec) / 1000000;
        if (ms >= timeout_ms) return 0;
    }
}

static void ser_write(const unsigned char *b, int n) { write(ser_fd, b, n); }

/* ---- Memoria del PIC: diccionario simple de direccion->byte ---- */
static unsigned char pic_mem[2 * MAXFLASH];
static unsigned char pic_set[2 * MAXFLASH];  /* marca posiciones escritas */

static int load_hex(const char *path) {
    FILE *f = fopen(path, "r");
    if (!f) { perror(path); return -1; }
    char line[600];
    while (fgets(line, sizeof line, f)) {
        if (line[0] != ':') continue;
        char tmp[3] = {0};
        tmp[0]=line[1]; tmp[1]=line[2]; int count = (int)strtol(tmp,0,16);
        char adr[5] = {0};
        strncpy(adr, line+3, 4); long address = strtol(adr,0,16);
        tmp[0]=line[7]; tmp[1]=line[8]; int rtype = (int)strtol(tmp,0,16);
        /* parar antes de config (0x2007*2 = 0x400E) y eeprom */
        if (strncmp(line, ":020000040030", 13) == 0) break;
        if (strncmp(line, ":0200000400F0", 13) == 0) break;
        if (rtype == 0) {
            for (int k = 0; k < count; k++) {
                tmp[0]=line[9+2*k]; tmp[1]=line[10+2*k];
                int data = (int)strtol(tmp,0,16);
                long a = address + k;
                if (a < 0x08) {
                    long hi = a + 2*MAXFLASH - 2*BSIZE;  /* reubicacion vector */
                    if (hi >= 0 && hi < 2*MAXFLASH) { pic_mem[hi]=data; pic_set[hi]=1; }
                    pic_mem[a]=data; pic_set[a]=1;
                } else if (a < 2*MAXFLASH) {
                    pic_mem[a]=data; pic_set[a]=1;
                }
            }
        }
    }
    fclose(f);
    /* vector de reset -> salto al bootloader (logica TinyBld 16F8XX) */
    pic_mem[0]=0x1f; pic_set[0]=1;  pic_mem[1]=0x30; pic_set[1]=1;
    pic_mem[2]=0x8a; pic_set[2]=1;  pic_mem[3]=0x00; pic_set[3]=1;
    pic_mem[4]=0xa0; pic_set[4]=1;  pic_mem[5]=0x2F; pic_set[5]=1;
    return 0;
}

static int detect(void) {
    printf(">>> Tienes 25 s. PULSA (y suelta) RESET en la placa, con calma.\n");
    unsigned char c = 0xC1, r;
    struct timespec start, now; clock_gettime(CLOCK_MONOTONIC,&start);
    for (;;) {
        ser_write(&c, 1);
        if (ser_read_byte(&r, 50)) {
            unsigned char r2;
            if (ser_read_byte(&r2, 50) && r2 == 'K') {
                printf(" Encontrado PIC, tipo = 0x%02X%s\n", r,
                       r==PIC_TYPE ? "  (16F876A/877A reconocido)" : "");
                return 1;
            }
        }
        clock_gettime(CLOCK_MONOTONIC,&now);
        long s = now.tv_sec - start.tv_sec;
        if (s >= 25) break;
    }
    printf(" No se detecto el bootloader. Revisa cable y pulsa Reset al correr.\n");
    return 0;
}

static int write_block(long pic_pos, unsigned char *blk, int rl) {
    unsigned char hm=(pic_pos/256)&255, lm=pic_pos&255;
    int chs = hm+lm+rl;
    unsigned char hdr[3]={hm,lm,(unsigned char)rl};
    ser_write(hdr,3);
    for (int k=0;k<rl;k++){ chs+=blk[k]; ser_write(&blk[k],1); }
    unsigned char cks=(unsigned char)((-chs)&255);
    ser_write(&cks,1);
    unsigned char ret;
    if (!ser_read_byte(&ret, 2000) || ret!='K') return 0;
    return 1;
}

static int program(void) {
    int hblock=8, block=4;
    long maxpos=MAXFLASH-BSIZE+4;
    printf("Escribiendo...\n");
    for (long pic_pos=0; pic_pos<maxpos; pic_pos+=block) {
        unsigned char blk[8]; int wr=0;
        for (int j=0;j<hblock;j++){
            long hp=2*pic_pos+j;
            if (hp<2*MAXFLASH && pic_set[hp]){ blk[j]=pic_mem[hp]; wr=1; }
            else blk[j]=0xFF;
        }
        if (wr) {
            if (!write_block(pic_pos, blk, hblock)) {
                printf("  ERROR en 0x%04lX\n", pic_pos); return 0;
            }
        }
    }
    printf("Grabado OK.\n");
    return 1;
}

int main(int argc, char **argv) {
    const char *port=NULL, *hex=NULL; int do_detect=0;
    for (int i=1;i<argc;i++){
        if (!strcmp(argv[i],"-p")&&i+1<argc) port=argv[++i];
        else if (!strcmp(argv[i],"-f")&&i+1<argc) hex=argv[++i];
        else if (!strcmp(argv[i],"--detect")) do_detect=1;
    }
    if (!port){ fprintf(stderr,"uso: tinybld_load -p PUERTO [-f hex | --detect]\n"); return 1; }

    ser_fd = ser_open(port);
    if (ser_fd<0){ fprintf(stderr,"No se pudo abrir %s\n",port); return 1; }
    printf("Abriendo %s a 115200...\n", port);

    int rc = 0;
    if (do_detect) {
        rc = detect() ? 0 : 1;
    } else if (hex) {
        if (!detect()) { close(ser_fd); return 1; }
        printf("Leyendo %s ...\n", hex);
        memset(pic_set,0,sizeof pic_set);
        if (load_hex(hex)!=0){ close(ser_fd); return 1; }
        rc = program() ? 0 : 1;
    } else {
        fprintf(stderr,"Falta -f hex o --detect\n"); rc=1;
    }
    close(ser_fd);
    return rc;
}

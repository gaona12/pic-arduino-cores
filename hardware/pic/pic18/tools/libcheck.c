/*
 * libcheck.c - Verificador de compatibilidad de librerias por chip.
 *
 * Dado un chip (mcpu) y un manifiesto .compat, dice si la libreria es
 * compatible, incompatible (y por que) o advertencia. Parte del ecosistema C.
 *
 * Uso:  libcheck <mcpu> <ruta.compat>
 * Salida: imprime estado y devuelve 0 si compatible, 1 si no.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

/* Capacidades de cada chip (que perifericos tiene). Tabla curada. */
typedef struct {
    const char *chip;
    int has_mssp;   /* I2C/SPI */
    int has_adc;
    int has_pwm;    /* CCP */
    int has_uart;
    int ram;        /* bytes */
} ChipCaps;

static const ChipCaps CHIPS[] = {
    /* chip        mssp adc pwm uart  ram */
    {"16F877A",    1,  1,  1,  1,  368},
    {"16F887",     1,  1,  1,  1,  368},
    {"16F886",     1,  1,  1,  1,  368},
    {"16F876A",    1,  1,  1,  1,  368},
    {"16F874A",    1,  1,  1,  1,  192},
    {"16F873A",    1,  1,  1,  1,  192},
    {"16F88",      1,  1,  1,  1,  368},
    {"16F628A",    0,  0,  1,  1,  224},
    {"16F84A",     0,  0,  0,  0,   68},
    {"16F690",     1,  1,  1,  1,  256},
    /* PIC18 */
    {"18F4550",    1,  1,  1,  1, 2048},
    {"18F2550",    1,  1,  1,  1, 2048},
    {"18F4455",    1,  1,  1,  1, 2048},
    {"18F2455",    1,  1,  1,  1, 2048},
    {"18F4520",    1,  1,  1,  1, 1536},
    {"18F2520",    1,  1,  1,  1, 1536},
    {"18F4620",    1,  1,  1,  1, 3968},
    {"18F2620",    1,  1,  1,  1, 3968},
    {"18F4525",    1,  1,  1,  1, 3968},
    {"18F452",     1,  1,  1,  1, 1536},
    {"18F458",     1,  1,  1,  1, 1536},
    {"18F4580",    1,  1,  1,  1, 1536},
    {NULL,0,0,0,0,0}
};

static const ChipCaps *find_chip(const char *mcpu) {
    for (const ChipCaps *c = CHIPS; c->chip; c++)
        if (strcasecmp(c->chip, mcpu) == 0) return c;
    return NULL;
}

/* lee un valor del manifiesto por clave */
static int get_val(const char *path, const char *key, char *out, int outsz) {
    FILE *f = fopen(path, "r");
    if (!f) return 0;
    char line[512]; int klen = strlen(key); int found = 0;
    while (fgets(line, sizeof line, f)) {
        if (line[0]=='#') continue;
        if (strncmp(line, key, klen)==0 && line[klen]=='=') {
            char *v = line + klen + 1;
            /* cortar en comentario inline o newline */
            char *h = strchr(v, '#'); if (h) *h = 0;
            char *nl = strpbrk(v, "\r\n"); if (nl) *nl = 0;
            /* trim */
            while (*v==' '||*v=='\t') v++;
            int n = strlen(v); while (n>0 && (v[n-1]==' '||v[n-1]=='\t')) v[--n]=0;
            strncpy(out, v, outsz-1); out[outsz-1]=0; found=1; break;
        }
    }
    fclose(f);
    return found;
}

int main(int argc, char **argv) {
    if (argc != 3) { fprintf(stderr, "uso: libcheck <mcpu> <ruta.compat>\n"); return 2; }
    const char *mcpu = argv[1];
    const char *compat = argv[2];

    const ChipCaps *c = find_chip(mcpu);
    if (!c) { printf("AVISO: chip %s desconocido para el verificador.\n", mcpu); return 0; }

    char name[128]="?", requires[64]="", excl[256]="", minram[16]="0";
    get_val(compat, "name", name, sizeof name);
    get_val(compat, "requires", requires, sizeof requires);
    get_val(compat, "chips_excluidos", excl, sizeof excl);
    get_val(compat, "min_ram", minram, sizeof minram);

    /* 1) chip excluido explicitamente? */
    if (excl[0]) {
        char buf[256]; strncpy(buf, excl, sizeof buf -1); buf[sizeof buf-1]=0;
        for (char *tok = strtok(buf, ", "); tok; tok = strtok(NULL, ", ")) {
            if (strcasecmp(tok, mcpu) == 0) {
                printf("INCOMPATIBLE: '%s' no soporta el %s (excluido explicitamente).\n", name, mcpu);
                return 1;
            }
        }
    }

    /* 2) requisito de periferico */
    if (strcasecmp(requires, "MSSP") == 0 && !c->has_mssp) {
        printf("INCOMPATIBLE: '%s' necesita I2C/SPI (MSSP) y el %s no lo tiene.\n", name, mcpu);
        return 1;
    }
    if (strcasecmp(requires, "ADC") == 0 && !c->has_adc) {
        printf("INCOMPATIBLE: '%s' necesita ADC y el %s no lo tiene.\n", name, mcpu);
        return 1;
    }
    if (strcasecmp(requires, "PWM") == 0 && !c->has_pwm) {
        printf("INCOMPATIBLE: '%s' necesita PWM (CCP) y el %s no lo tiene.\n", name, mcpu);
        return 1;
    }
    if (strcasecmp(requires, "UART") == 0 && !c->has_uart) {
        printf("INCOMPATIBLE: '%s' necesita UART y el %s no lo tiene.\n", name, mcpu);
        return 1;
    }

    /* 3) RAM minima */
    int need = atoi(minram);
    if (need > c->ram) {
        printf("ADVERTENCIA: '%s' pide ~%d B RAM; el %s tiene %d B. Puede no caber.\n",
               name, need, mcpu, c->ram);
        return 0; /* advertencia, no bloquea */
    }

    printf("OK: '%s' es compatible con el %s.\n", name, mcpu);
    return 0;
}

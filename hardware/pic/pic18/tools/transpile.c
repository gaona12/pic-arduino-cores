/*
 * transpile.c - Transpilador "Arduino Uno -> C (XC8)".
 *
 * NIVEL 1 (fijo): convierte la sintaxis de objeto de los objetos del core:
 *   Serial.begin/print/println/write/available/read
 *   lcd.begin/clear/home/setCursor/print/println/write
 *   Wire.begin/beginTransmission/write/endTransmission/requestFrom/read
 *   SPI.begin/transfer
 *   EEPROM.read/write
 * Para print/println detecta si el primer argumento es TEXTO o NUMERO.
 *
 * NIVEL 2 (dinamico, por libreria): lee los archivos .map de pic_libraries y
 * aprende objetos declarados por el usuario para traducir su sintaxis:
 *   Servo miServo;              -> (registra miServo:Servo; opcional ctor)
 *   miServo.attach(9);          -> Servo_attach(9);
 *   miServo.write(90);          -> Servo_write(90);
 * Un .map declara:  class=Servo  prefix=Servo_  [ctor=Servo_init]
 *
 * Uso:  transpile entrada.cpp salida.c [dir_pic_libraries]
 *       (el 3er argumento es opcional; sin el, solo actua el Nivel 1)
 *
 * Parte del ecosistema en C del core (sin Python).
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>

/* ------- utilidades de buffer dinamico ------- */
typedef struct { char *buf; size_t len, cap; } Str;
static void sinit(Str *s){ s->cap=1024; s->len=0; s->buf=malloc(s->cap); s->buf[0]=0; }
static void sput(Str *s, const char *p, size_t n){
    if (s->len+n+1 > s->cap){ while(s->len+n+1>s->cap) s->cap*=2; s->buf=realloc(s->buf,s->cap); }
    memcpy(s->buf+s->len, p, n); s->len+=n; s->buf[s->len]=0;
}
static void sputs(Str *s, const char *p){ sput(s,p,strlen(p)); }
static void sputc(Str *s, char c){ sput(s,&c,1); }

/* ------- mapa de metodos simples: "obj.metodo" -> "funcion" ------- */
typedef struct { const char *from; const char *to; } Map;
static const Map SIMPLE[] = {
    {"Serial.begin",            "Serial_begin"},
    {"Serial.write",            "Serial_write"},
    {"Serial.available",        "Serial_available"},
    {"Serial.read",             "Serial_read"},
    {"lcd.clear",               "lcd_clear"},
    {"lcd.home",                "lcd_home"},
    {"lcd.setCursor",           "lcd_setCursor"},
    {"lcd.write",               "lcd_write"},
    {"lcd.command",             "lcd_command"},
    {"Wire.beginTransmission",  "Wire_beginTransmission"},
    {"Wire.endTransmission",    "Wire_endTransmission"},
    {"Wire.requestFrom",        "Wire_requestFrom"},
    {"Wire.begin",              "Wire_begin"},
    {"Wire.write",              "Wire_write"},
    {"Wire.read",               "Wire_read"},
    {"SPI.begin",               "SPI_begin"},
    {"SPI.transfer",            "SPI_transfer"},
    {"EEPROM.read",             "EEPROM_read"},
    {"EEPROM.write",            "EEPROM_write"},
    {NULL,NULL}
};

/* print/println: objeto -> (fn texto, fn texto ln, fn num, fn num ln) */
typedef struct {
    const char *obj;
    const char *p_txt, *pln_txt, *p_num, *pln_num;
} PrintMap;
static const PrintMap PRINTS[] = {
    {"Serial", "Serial_print", "Serial_println", "Serial_printInt", "Serial_printIntln"},
    {"lcd",    "lcd_print",    "lcd_println",    "lcd_printInt",    "lcd_printIntln"},
    {NULL,NULL,NULL,NULL,NULL}
};

static int is_ident_char(char c){
    return (c=='_' || (c>='A'&&c<='Z') || (c>='a'&&c<='z') || (c>='0'&&c<='9'));
}
static int is_ident_start(char c){
    return (c=='_' || (c>='A'&&c<='Z') || (c>='a'&&c<='z'));
}

/* ===================== NIVEL 2: mapas de librerias ===================== */
#define MAXLIB 64
#define MAXOBJ 256
#define NAMELEN 64

typedef struct {
    char cls[NAMELEN];     /* nombre de la clase: "Servo"            */
    char prefix[NAMELEN];  /* prefijo C: "Servo_"                    */
    char ctor[NAMELEN];    /* funcion del constructor (o vacio)      */
} LibMap;

typedef struct {
    char name[NAMELEN];    /* nombre del objeto del usuario: "miServo" */
    int  lib;              /* indice en LIBS                           */
} ObjDecl;

static LibMap  LIBS[MAXLIB];  static int NLIB=0;
static ObjDecl OBJS[MAXOBJ];  static int NOBJ=0;

static void trim(char *s){
    char *p=s; while(*p==' '||*p=='\t') p++;
    if(p!=s) memmove(s,p,strlen(p)+1);
    size_t n=strlen(s);
    while(n>0 && (s[n-1]==' '||s[n-1]=='\t'||s[n-1]=='\r'||s[n-1]=='\n')) s[--n]=0;
}

/* lee un .map y lo agrega a LIBS */
static void load_map(const char *path){
    FILE *f=fopen(path,"r"); if(!f) return;
    if(NLIB>=MAXLIB){ fclose(f); return; }
    LibMap m; m.cls[0]=m.prefix[0]=m.ctor[0]=0;
    char line[256];
    while(fgets(line,sizeof line,f)){
        char *h=strchr(line,'#'); if(h) *h=0;      /* quita comentarios */
        char *eq=strchr(line,'='); if(!eq) continue;
        *eq=0; char *key=line; char *val=eq+1;
        trim(key); trim(val);
        if(!*val) continue;
        if(!strcmp(key,"class"))  { strncpy(m.cls,val,NAMELEN-1);    m.cls[NAMELEN-1]=0; }
        else if(!strcmp(key,"prefix")){ strncpy(m.prefix,val,NAMELEN-1); m.prefix[NAMELEN-1]=0; }
        else if(!strcmp(key,"ctor")){ strncpy(m.ctor,val,NAMELEN-1); m.ctor[NAMELEN-1]=0; }
    }
    fclose(f);
    if(m.cls[0] && m.prefix[0]) LIBS[NLIB++]=m;
}

/* recorre dir_pic_libraries/<lib>/<lib>.map y carga todos los mapas */
static void load_all_maps(const char *libdir){
    DIR *d=opendir(libdir); if(!d) return;
    struct dirent *e;
    while((e=readdir(d))){
        if(e->d_name[0]=='.') continue;
        char sub[1024];
        snprintf(sub,sizeof sub,"%s/%s",libdir,e->d_name);
        DIR *d2=opendir(sub); if(!d2) continue;      /* solo subcarpetas */
        struct dirent *e2;
        while((e2=readdir(d2))){
            size_t ln=strlen(e2->d_name);
            if(ln>4 && !strcmp(e2->d_name+ln-4,".map")){
                char mp[2048];
                snprintf(mp,sizeof mp,"%s/%s",sub,e2->d_name);
                load_map(mp);
            }
        }
        closedir(d2);
    }
    closedir(d);
}

static int find_lib_by_class(const char *cls){
    for(int i=0;i<NLIB;i++) if(!strcmp(LIBS[i].cls,cls)) return i;
    return -1;
}
static int find_obj(const char *name){
    for(int i=0;i<NOBJ;i++) if(!strcmp(OBJS[i].name,name)) return i;
    return -1;
}

/* ===================================================================== */

/* Lee el archivo completo a memoria */
static char *read_file(const char *path, size_t *out_len){
    FILE *f=fopen(path,"rb");
    if(!f){ perror(path); return NULL; }
    fseek(f,0,SEEK_END); long n=ftell(f); fseek(f,0,SEEK_SET);
    char *b=malloc(n+1); fread(b,1,n,f); b[n]=0; fclose(f);
    *out_len=(size_t)n; return b;
}

/* Encuentra el ')' que cierra el '(' en src[open], y marca si el primer arg
 * (en el nivel 1 de parentesis) es texto (empieza con comilla doble). */
static size_t scan_call(const char *s, size_t open, int *first_is_text){
    size_t i=open; int depth=0; int in_str=0, esc=0; char q=0;
    int seen_nonspace=0; int firsttext=0;
    for(; s[i]; i++){
        char c=s[i];
        if(in_str){
            if(esc) esc=0; else if(c=='\\') esc=1; else if(c==q) in_str=0;
        } else {
            if(c=='"'||c=='\''){
                if(!seen_nonspace && depth==1){ firsttext=(c=='"'); }
                in_str=1; q=c; seen_nonspace=1;
            } else if(c=='('){ depth++; }
            else if(c==')'){ depth--; if(depth==0){ *first_is_text=firsttext; return i; } }
            else if(c!=' '&&c!='\t'){
                if(!seen_nonspace && depth==1){ firsttext=0; seen_nonspace=1; }
            }
        }
    }
    *first_is_text=firsttext; return i;
}

/* ---- PASADA 1: descubrir declaraciones "Clase obj;" / "Clase obj(args);" ----
 * Solo para clases que estan en LIBS. Registra obj->lib en OBJS.
 * Es un reconocedor sencillo: identificador(clase) + espacios + identificador(obj)
 * + (opcional "(...)") + ";". Se salta cadenas y comentarios. */
static void discover_objects(const char *s, size_t n){
    size_t i=0; int in_str=0,esc=0; char q=0; int in_lc=0,in_bc=0;
    for(i=0;i<n;){
        char c=s[i];
        if(in_lc){ if(c=='\n') in_lc=0; i++; continue; }
        if(in_bc){ if(c=='*'&&s[i+1]=='/'){ in_bc=0; i+=2; } else i++; continue; }
        if(in_str){ if(esc) esc=0; else if(c=='\\') esc=1; else if(c==q) in_str=0; i++; continue; }
        if(c=='/'&&s[i+1]=='/'){ in_lc=1; i+=2; continue; }
        if(c=='/'&&s[i+1]=='*'){ in_bc=1; i+=2; continue; }
        if(c=='"'||c=='\''){ in_str=1; q=c; i++; continue; }

        if(is_ident_start(c) && (i==0 || !is_ident_char(s[i-1]))){
            size_t k=i; while(s[k]&&is_ident_char(s[k])) k++;
            char word[NAMELEN]; size_t wl=k-i;
            if(wl<NAMELEN){ memcpy(word,s+i,wl); word[wl]=0;
                int lib=find_lib_by_class(word);
                if(lib>=0){
                    /* espacios, luego el nombre del objeto */
                    size_t j=k; while(s[j]==' '||s[j]=='\t') j++;
                    if(is_ident_start(s[j])){
                        size_t o=j; while(s[j]&&is_ident_char(s[j])) j++;
                        char obj[NAMELEN]; size_t ol=j-o;
                        if(ol<NAMELEN){ memcpy(obj,s+o,ol); obj[ol]=0;
                            size_t p=j; while(s[p]==' '||s[p]=='\t') p++;
                            /* debe seguir ';' o '(' para ser una declaracion */
                            if((s[p]==';'||s[p]=='(') && NOBJ<MAXOBJ && find_obj(obj)<0){
                                strncpy(OBJS[NOBJ].name,obj,NAMELEN-1);
                                OBJS[NOBJ].name[NAMELEN-1]=0;
                                OBJS[NOBJ].lib=lib; NOBJ++;
                            }
                        }
                    }
                }
            }
            i=k; continue;
        }
        i++;
    }
}

int main(int argc, char **argv){
    if(argc!=3 && argc!=4){
        fprintf(stderr,"uso: transpile entrada.cpp salida.c [dir_pic_libraries]\n");
        return 1;
    }
    size_t n; char *src=read_file(argv[1],&n);
    if(!src) return 1;

    /* Nivel 2: cargar mapas de librerias y descubrir objetos del usuario */
    if(argc==4){
        load_all_maps(argv[3]);
        discover_objects(src,n);
    }

    Str out; sinit(&out);
    size_t i=0;
    int in_str=0, esc=0; char q=0;
    int in_lc=0, in_bc=0;

    while(i<n){
        char c=src[i];

        if(in_lc){ sputc(&out,c); if(c=='\n') in_lc=0; i++; continue; }
        if(in_bc){ sputc(&out,c); if(c=='*'&&src[i+1]=='/'){ sputc(&out,'/'); i+=2; in_bc=0; } else i++; continue; }
        if(in_str){ sputc(&out,c); if(esc) esc=0; else if(c=='\\') esc=1; else if(c==q) in_str=0; i++; continue; }

        if(c=='/'&&src[i+1]=='/'){ in_lc=1; sputc(&out,c); i++; continue; }
        if(c=='/'&&src[i+1]=='*'){ in_bc=1; sputc(&out,c); i++; continue; }
        if(c=='"'||c=='\''){ in_str=1; q=c; sputc(&out,c); i++; continue; }

        if(is_ident_char(c) && (i==0 || !is_ident_char(src[i-1]))){
            int matched=0;

            /* ---- Nivel 2a: declaracion de objeto de libreria ---- */
            /* "Clase obj;"  o  "Clase obj(args);"  */
            if(NLIB>0){
                size_t k=i; while(src[k]&&is_ident_char(src[k])) k++;
                char word[NAMELEN]; size_t wl=k-i;
                if(wl<NAMELEN){ memcpy(word,src+i,wl); word[wl]=0;
                    int lib=find_lib_by_class(word);
                    if(lib>=0){
                        size_t j=k; while(src[j]==' '||src[j]=='\t') j++;
                        if(is_ident_start(src[j])){
                            size_t o=j; while(src[j]&&is_ident_char(src[j])) j++;
                            char obj[NAMELEN]; size_t ol=j-o;
                            if(ol<NAMELEN){ memcpy(obj,src+o,ol); obj[ol]=0; } else obj[0]=0;
                            size_t p=j; while(src[p]==' '||src[p]=='\t') p++;
                            if(ol<NAMELEN && (src[p]==';'||src[p]=='(') && find_obj(obj)>=0){
                                /* es una declaracion registrada: traducir */
                                const LibMap *m=&LIBS[lib];
                                if(src[p]=='('){
                                    /* Clase obj(args); -> ctor(args);  (o se omite) */
                                    int dummy; size_t close=scan_call(src,p,&dummy);
                                    if(m->ctor[0]){
                                        sputs(&out,m->ctor);
                                        sput(&out,src+p,close-p+1); /* (args) */
                                    } else {
                                        /* sin ctor: dejar comentario y nada ejecutable */
                                        sputs(&out,"/* ");
                                        sput(&out,src+i,close-i+1);
                                        sputs(&out," */");
                                    }
                                    i=close+1;
                                    while(src[i]==' '||src[i]=='\t') { sputc(&out,src[i]); i++; }
                                    if(src[i]==';'){ i++; } /* consumir el ';' de la decl */
                                    matched=1;
                                } else {
                                    /* Clase obj;  (sin parentesis) */
                                    if(m->ctor[0]){
                                        sputs(&out,m->ctor); sputs(&out,"()");
                                    } else {
                                        sputs(&out,"/* "); sput(&out,src+i,p-i); sputs(&out," */");
                                    }
                                    i=p; /* el ';' se copia normal despues */
                                    matched=1;
                                }
                            }
                        }
                    }
                }
            }
            if(matched) continue;

            /* ---- Nivel 2b: uso de metodo de objeto de libreria ----
             * obj.metodo(...)  ->  Prefijo_metodo(...)                */
            if(NOBJ>0){
                size_t k=i; while(src[k]&&is_ident_char(src[k])) k++;
                size_t wl=k-i;
                if(wl<NAMELEN && src[k]=='.'){
                    char nm[NAMELEN]; memcpy(nm,src+i,wl); nm[wl]=0;
                    int oi=find_obj(nm);
                    if(oi>=0){
                        /* leer el nombre del metodo */
                        size_t mstart=k+1, me=mstart;
                        while(src[me]&&is_ident_char(src[me])) me++;
                        if(me>mstart){
                            const LibMap *m=&LIBS[OBJS[oi].lib];
                            sputs(&out,m->prefix);           /* "Servo_"   */
                            sput(&out,src+mstart,me-mstart); /* "attach"   */
                            i=me; matched=1;                 /* deja "(args)" al flujo normal */
                        }
                    }
                }
            }
            if(matched) continue;

            /* ---- Nivel 1: print/println con deteccion de tipo ---- */
            for(const PrintMap *pm=PRINTS; pm->obj && !matched; pm++){
                for(int ln=0; ln<2 && !matched; ln++){
                    char pat[48];
                    snprintf(pat,sizeof pat,"%s.%s", pm->obj, ln? "println":"print");
                    size_t pl=strlen(pat);
                    if(strncmp(src+i,pat,pl)==0 && !is_ident_char(src[i+pl])){
                        size_t j=i+pl; while(src[j]==' '||src[j]=='\t') j++;
                        if(src[j]=='('){
                            int is_text=0;
                            size_t close=scan_call(src,j,&is_text);
                            const char *fn = ln ? (is_text?pm->pln_txt:pm->pln_num)
                                                : (is_text?pm->p_txt :pm->p_num);
                            sputs(&out,fn);
                            sput(&out, src+j, close-j+1);
                            i=close+1; matched=1;
                        }
                    }
                }
            }
            if(matched) continue;

            /* ---- Nivel 1: metodos simples del core ---- */
            for(const Map *m=SIMPLE; m->from; m++){
                size_t fl=strlen(m->from);
                if(strncmp(src+i,m->from,fl)==0 && !is_ident_char(src[i+fl])){
                    sputs(&out,m->to);
                    i+=fl; matched=1; break;
                }
            }
            if(matched) continue;

            /* lcd.begin(...) -> lcd_begin() */
            if(strncmp(src+i,"lcd.begin",9)==0 && !is_ident_char(src[i+9])){
                size_t j=i+9; while(src[j]==' '||src[j]=='\t') j++;
                if(src[j]=='('){
                    int dummy; size_t close=scan_call(src,j,&dummy);
                    sputs(&out,"lcd_begin()");
                    i=close+1; continue;
                }
            }

            /* identificador normal: copiar entero */
            size_t k=i; while(src[k] && is_ident_char(src[k])) k++;
            sput(&out, src+i, k-i);
            i=k; continue;
        }

        sputc(&out,c); i++;
    }

    FILE *fo=fopen(argv[2],"wb");
    if(!fo){ perror(argv[2]); return 1; }
    fwrite(out.buf,1,out.len,fo); fclose(fo);
    free(src); free(out.buf);
    return 0;
}

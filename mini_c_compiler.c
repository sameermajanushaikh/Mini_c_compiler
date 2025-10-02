
/* mini_c_compiler.c - Simple Mini C Compiler (lexer + parser + 3-address code generator) */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
enum TokenType { TOK_EOF, TOK_ID, TOK_NUM, TOK_IF, TOK_ELSE, TOK_WHILE,
TOK_SEMI, TOK_ASSIGN, TOK_PLUS, TOK_MINUS, TOK_MUL, TOK_DIV,
TOK_LPAREN, TOK_RPAREN, TOK_LBRACE, TOK_RBRACE,
TOK_LT, TOK_GT, TOK_LE, TOK_GE, TOK_EQ, TOK_NE };
const char *tok_names[] = {"EOF","ID","NUM","if","else","while",";","=","+","-","*","/","("," )","{","}","<",">","<=",">=","==","!="};
typedef struct { enum TokenType type; char lexeme[128]; } Token;
static const char *src;
static int pos;
static Token curtok;
void lexer_init(const char *s) { src = s; pos = 0; }
char peekch() { return src[pos]; }
char readch() { return src[pos]==0?0:src[pos++]; }
void skip_ws() { while(isspace(peekch())) readch(); }
Token make_token(enum TokenType t, const char *lex) { Token tk; tk.type = t; strncpy(tk.lexeme, lex, 127); tk.lexeme[127]='\0'; return tk; }
Token next_token() {
    skip_ws(); char c = peekch(); if(c==0) return make_token(TOK_EOF,"<EOF>");
    if(isalpha(c) || c=='_') { char buf[128]; int i=0; while(isalnum(peekch())||peekch()=='_'){ buf[i++]=readch(); if(i>=127) break;} buf[i]='\0'; if(strcmp(buf,"if")==0) return make_token(TOK_IF,buf); if(strcmp(buf,"else")==0) return make_token(TOK_ELSE,buf); if(strcmp(buf,"while")==0) return make_token(TOK_WHILE,buf); return make_token(TOK_ID,buf); }
    if(isdigit(c)) { char buf[128]; int i=0; while(isdigit(peekch())){ buf[i++]=readch(); if(i>=127) break;} buf[i]='\0'; return make_token(TOK_NUM,buf); }
    if(c=='='){ readch(); if(peekch()=='='){ readch(); return make_token(TOK_EQ,"==");} return make_token(TOK_ASSIGN,"=");}
    if(c==';'){ readch(); return make_token(TOK_SEMI,";");}
    if(c=='+'){ readch(); return make_token(TOK_PLUS,"+");}
    if(c=='-'){ readch(); return make_token(TOK_MINUS,"-");}
    if(c=='*'){ readch(); return make_token(TOK_MUL,"*");}
    if(c=='/'){ readch(); return make_token(TOK_DIV,"/");}
    if(c=='('){ readch(); return make_token(TOK_LPAREN,"(");}
    if(c==')'){ readch(); return make_token(TOK_RPAREN,")");}
    if(c=='{'){ readch(); return make_token(TOK_LBRACE,"{");}
    if(c=='}'){ readch(); return make_token(TOK_RBRACE,"}");}
    if(c=='<'){ readch(); if(peekch()=='='){readch(); return make_token(TOK_LE,"<=");} return make_token(TOK_LT,"<");}
    if(c=='>'){ readch(); if(peekch()=='='){readch(); return make_token(TOK_GE,">=");} return make_token(TOK_GT,">");}
    if(c=='!'){ readch(); if(peekch()=='='){readch(); return make_token(TOK_NE,"!=");}}
    readch(); return make_token(TOK_EOF,"<EOF>");
}
void advance() { curtok = next_token(); }
static int temp_cnt=0, label_cnt=0;
char* newtemp(){ char *s=malloc(16); sprintf(s,"t%d",++temp_cnt); return s;}
char* newlabel(){ char *s=malloc(16); sprintf(s,"L%d",++label_cnt); return s;}
char *parse_expr(); char *parse_term(); char *parse_factor(); char *parse_stmt(); char *parse_stmt_list();
char *parse_factor(){ if(curtok.type==TOK_LPAREN){ advance(); char *res=parse_expr(); if(curtok.type!=TOK_RPAREN){printf("Error: ) expected\n"); exit(1);} advance(); return res;} if(curtok.type==TOK_NUM){ char *t=newtemp(); printf("%s = %s\n",t,curtok.lexeme); advance(); return t;} if(curtok.type==TOK_ID){ char *t=strdup(curtok.lexeme); advance(); return t;} printf("Syntax error: factor expected\n"); exit(1); }
char *parse_term(){ char *t=parse_factor(); while(curtok.type==TOK_MUL || curtok.type==TOK_DIV){ char op=curtok.type==TOK_MUL?'*':'/'; advance(); char *f=parse_factor(); char *tmp=newtemp(); printf("%s = %s %c %s\n",tmp,t,op,f); t=tmp;} return t;}
char *parse_expr(){ char *t=parse_term(); while(curtok.type==TOK_PLUS || curtok.type==TOK_MINUS){ char op=curtok.type==TOK_PLUS?'+':'-'; advance(); char *f=parse_term(); char *tmp=newtemp(); printf("%s = %s %c %s\n",tmp,t,op,f); t=tmp;} return t;}
void expect(enum TokenType tt){ if(curtok.type==tt) advance(); else {printf("Syntax error: expected %d but got %d ('%s')\n", tt, curtok.type, curtok.lexeme); exit(1);} }
char *parse_stmt(){ if(curtok.type==TOK_ID){ char id[128]; strcpy(id,curtok.lexeme); advance(); expect(TOK_ASSIGN); char *rhs=parse_expr(); expect(TOK_SEMI); printf("%s = %s\n",id,rhs); return NULL;} if(curtok.type==TOK_IF){ advance(); expect(TOK_LPAREN); char *cond=parse_expr(); expect(TOK_RPAREN); char *L1=newlabel(),*L2=newlabel(); printf("if %s == 0 goto %s\n",cond,L1); parse_stmt(); printf("%s:\n",L1); if(curtok.type==TOK_ELSE){ advance(); parse_stmt();} return NULL;} if(curtok.type==TOK_WHILE){ advance(); expect(TOK_LPAREN); char *Lstart=newlabel(); char *Lend=newlabel(); printf("%s:\n",Lstart); char *cond=parse_expr(); expect(TOK_RPAREN); printf("if %s == 0 goto %s\n",cond,Lend); parse_stmt(); printf("goto %s\n%s:\n",Lstart,Lend); return NULL;} if(curtok.type==TOK_LBRACE){ advance(); while(curtok.type!=TOK_RBRACE && curtok.type!=TOK_EOF) parse_stmt(); expect(TOK_RBRACE); return NULL;} printf("Syntax error: statement expected\n"); exit(1);}
char *parse_stmt_list(){ while(curtok.type!=TOK_EOF) parse_stmt(); return NULL;}
int main(){ char code[1024]; printf("Enter C-like code (single line, end with Ctrl+Z):\n"); fread(code,1,sizeof(code),stdin); lexer_init(code); advance(); parse_stmt_list(); return 0;}

// pl/0 compiler with code generation
#include <stdlib.h>
#include <string.h>
#include "pl0.h"

void error(long n){
    long i;

    printf(" ****");
    for (i=1; i<=cc-1; i++){
	printf(" ");
    }
    printf("^%2d\n",n);
    err++;
}

//从输入文件中获取下一个字符
void getch() {
	//cc与ll相等的情况：这一行第一次获取时，将一行中的代码放置到line数组当中，此处，line数组相当于缓存
	//cc与ll不相等的情况：这个函数在第一遍执行完之后，只是将一行代码缓存到line数组当中，真正返回的信息只是ch全局变量
	//                  相当于返回一个字符，所以还需要获取这一行后面的字符，此时，就跳过这个判断，不用重新读取文件，
	//                  直接从line缓冲区中获取即可
    if(cc==ll){
		//feof是检测流上的文件结束符，如果文件结束，则返回非0值，否则返回0
		//（即，文件结束：返回非0值，文件未结束，返回0值）
		//判断文件是否结束
		if(feof(infile))
		{
			// the end of file 
			printf("************************************\n");
			printf("      program incomplete\n");
			printf("************************************\n");
			exit(1);
		}
		//重新初始化ll和cc，为获取当前代码行做准备
		ll=0; cc=0;
		printf("%5d ", cx);
		//获取一行代码 - 调用一次getchar，即可获取一整行代码（即一个代码语句）
		while((!feof(infile))&&((ch=getc(infile))!='\n'))
		{
			printf("%c",ch);
			ll=ll+1; line[ll]=ch;
		}
		printf("\n");
		ll=ll+1; line[ll]=' ';
    }
	//只有当cc!=ll的时候，cc=cc+1才有意义，否则cc在循环中会被重置为0，这里永远是1，没有意义
	//如果一行代码成功获取，ch就是line中的第一个元素
	//如果再一次进入这个文件，ch从line缓冲区中获取下一个元素即可
    cc=cc+1; ch=line[cc];
}

//get symbol - 我猜这是词法分析
void getsym(){
    long i,j,k;

	//这里大概可能也许是从文件中提取出一行代码
    while(ch==' '||ch=='\t')
	{
		getch();
    }
	//isalpha判断字符是否为英文字母，若为英文字母，返回非0（小写字母为2，大写字母为1）。若不是字母，返回0。
    if(isalpha(ch))  // identified or reserved - 由字母开头，说明应该是标识符或者保留字
	{ 	
		k=0;
		do{
			if(k<al)
			{
				//话说这里应该判断a数组是否超出界限了啊，这不是典型的缓冲区溢出嘛！
				a[k]=ch; k=k+1;
			}
			getch();  //获取下一个字符
		}while(isalpha(ch)||isdigit(ch));  //如果继续是字符或者数字，说明这还是标识符，所以继续
		
		//kk有一个初始化，但是后续的kk是如何来的？？？
		if(k>=kk)
		{
			kk=k;
		}
		else  
		{
			do{
				//重新定义kk
				//补足a数组标识符后续位置
				kk=kk-1; a[kk]=' ';
			}while(k<kk);
		}
		//将a中存储的标识符存放到id数组当中
		strcpy(id,a); i=0; j=norw-1;
		do{
			k=(i+j)/2;

			if(strcmp(id,word[k])<=0)
			{
				j=k-1;
			}
			if(strcmp(id,word[k])>=0)
			{
				i=k+1;
			}
		}while(i<=j);
		if(i-1>j)
		{
			sym=wsym[k];
		}
		else
		{
			sym=ident;
		}
    }
	else if (isdigit(ch)) // number
	{
		k=0; num=0; sym=number;
		do
		{
			num=num*10+(ch-'0');
			k=k+1; getch();
		}while(isdigit(ch));
		if(k>nmax)
		{
			error(31);
		}
    }
	else if(ch==':')
	{
		getch();
		if (ch == '='){
			sym = becomes; getch();
		}
		else{
			sym = nul;
		}
    }
	else if(ch=='<')
	{
		getch();
		if(ch=='='){
			sym=leq; getch();
		}else if(ch=='>'){
			sym=neq; getch();
		}else{
			sym=lss;
		}
    }
	else if(ch=='>')
	{
		getch();
		if(ch=='='){
			sym=geq; getch();
		}else{
			sym=gtr;
		}
    }
	else
	{
		sym=ssym[(unsigned char)ch]; getch();
    }
}

void gen(enum fct x, long y, long z){
    if(cx>cxmax){
	printf("program too long\n");
	exit(1);
    }
    code[cx].f=x; code[cx].l=y; code[cx].a=z;
    cx=cx+1;
}

void test(unsigned long s1, unsigned long s2, long n){
    if (!(sym & s1)){
	error(n);
	s1=s1|s2;
	while(!(sym & s1)){
	    getsym();
	}
    }
}

void enter(enum object k){		// enter object into table
    tx=tx+1;
    strcpy(table[tx].name,id);
    table[tx].kind=k;
    switch(k){
	case constant:
	    if(num>amax){
		error(31);
		num = 0;
	    }
	    table[tx].val=num;
	    break;
	case variable:
	    table[tx].level=lev; table[tx].addr=dx; dx=dx+1;
	    break;
	case proc:
	    table[tx].level=lev;
	    break;
    }
}

long position(char* id){	// find identifier id in table
    long i;

    strcpy(table[0].name,id);
    i=tx;
    while(strcmp(table[i].name,id)!=0){
	i=i-1;
    }
    return i;
}

void constdeclaration(){
    if(sym==ident){
	getsym();
	if(sym==eql||sym==becomes){
	    if(sym==becomes){
		error(1);
	    }
	    getsym();
	    if(sym==number){
		enter(constant); getsym();
	    }else{
		error(2);
	    }
	}else{
	    error(3);
	}
    }else{
	error(4);
    }
}

void vardeclaration(){
    if(sym==ident){
	enter(variable); getsym();
    }else{
	error(4);
    }
}

void listcode(long cx0){	// list code generated for this block
    long i;

    for(i=cx0; i<=cx-1; i++){
	printf("%10d%5s%3d%5d\n", i, mnemonic[code[i].f], code[i].l, code[i].a);
    }

}

void expression(unsigned long);
void factor(unsigned long fsys){
    long i;
	
    test(facbegsys,fsys,24);
    while(sym & facbegsys){
	if(sym==ident){
	    i=position(id);
	    if(i==0){
		error(11);
	    }else{
		switch(table[i].kind){
		    case constant:
			gen(lit,0,table[i].val);
			break;
		    case variable:
			gen(lod,lev-table[i].level,table[i].addr);
			break;
		    case proc:
			error(21);
			break;
		}
	    }
	    getsym();
	}else if(sym==number){
	    if(num>amax){
		error(31); num=0;
	    }
	    gen(lit,0,num);
	    getsym();
	}else if(sym==lparen){
	    getsym();
	    expression(rparen|fsys);
	    if(sym==rparen){
		getsym();
	    }else{
		error(22);
	    }
	}
	test(fsys,lparen,23);
    }
}

void term(unsigned long fsys){
    unsigned long mulop;

    factor(fsys|times|slash);
    while(sym==times||sym==slash){
	mulop=sym; getsym();
	factor(fsys|times|slash);
	if(mulop==times){
	    gen(opr,0,4);
	}else{
	    gen(opr,0,5);
	}
    }
}

void expression(unsigned long fsys){
    unsigned long addop;

    if(sym==plus||sym==minus){
	addop=sym; getsym();
	term(fsys|plus|minus);
	if(addop==minus){
	    gen(opr,0,1);
	}
    }else{
	term(fsys|plus|minus);
    }
    while(sym==plus||sym==minus){
	addop=sym; getsym();
	term(fsys|plus|minus);
	if(addop==plus){
	    gen(opr,0,2);
	}else{
	    gen(opr,0,3);
	}
    }
}

void condition(unsigned long fsys){
    unsigned long relop;

    if(sym==oddsym){
	getsym(); expression(fsys);
	gen(opr,0,6);
    }else{
	expression(fsys|eql|neq|lss|gtr|leq|geq);
	if(!(sym&(eql|neq|lss|gtr|leq|geq))){
	    error(20);
	}else{
	    relop=sym; getsym();
	    expression(fsys);
	    switch(relop){
		case eql:
		    gen(opr,0,8);
		    break;
		case neq:
		    gen(opr,0,9);
		    break;
		case lss:
		    gen(opr,0,10);
		    break;
		case geq:
		    gen(opr,0,11);
		    break;
		case gtr:
		    gen(opr,0,12);
		    break;
		case leq:
		    gen(opr,0,13);
		    break;
	    }
	}
    }
}

void statement(unsigned long fsys){
    long i,cx1,cx2;

    if(sym==ident){
	i=position(id);
	if(i==0){
	    error(11);
	}else if(table[i].kind!=variable){	// assignment to non-variable
	    error(12); i=0;
	}
	getsym();
	if(sym==becomes){
	    getsym();
	}else{
	    error(13);
	}
	expression(fsys);
	if(i!=0){
	    gen(sto,lev-table[i].level,table[i].addr);
	}
    }else if(sym==callsym){
	getsym();
	if(sym!=ident){
	    error(14);
	}else{
	    i=position(id);
	    if(i==0){
		error(11);
	    }else if(table[i].kind==proc){
		gen(cal,lev-table[i].level,table[i].addr);
	    }else{
		error(15);
	    }
	    getsym();
	}
    }else if(sym==ifsym){
	getsym(); condition(fsys|thensym|dosym);
	if(sym==thensym){
	    getsym();
	}else{
	    error(16);
	}
	cx1=cx;	gen(jpc,0,0);
	statement(fsys);
	code[cx1].a=cx;	
    }else if(sym==beginsym){
	getsym(); statement(fsys|semicolon|endsym);
	while(sym==semicolon||(sym&statbegsys)){
	    if(sym==semicolon){
		getsym();
	    }else{
		error(10);
	    }
	    statement(fsys|semicolon|endsym);
	}
	if(sym==endsym){
	    getsym();
	}else{
	    error(17);
	}
    }else if(sym==whilesym){
	cx1=cx; getsym();
	condition(fsys|dosym);
	cx2=cx;	gen(jpc,0,0);
	if(sym==dosym){
	    getsym();
	}else{
	    error(18);
	}
	statement(fsys); gen(jmp,0,cx1);
	code[cx2].a=cx;
    }
    test(fsys,0,19);
}
			
void block(unsigned long fsys){
    long tx0;		// initial table index
    long cx0; 		// initial code index
    long tx1;		// save current table index before processing nested procedures
    long dx1;		// save data allocation index

    dx=3; tx0=tx; table[tx].addr=cx; gen(jmp,0,0);
    if(lev>levmax){
	error(32);
    }
    do{
	if(sym==constsym){
	    getsym();
	    do{
		constdeclaration();
		while(sym==comma){
		    getsym(); constdeclaration();
		}
		if(sym==semicolon){
		    getsym();
		}else{
		    error(5);
		}
	    }while(sym==ident);
	}
	if(sym==varsym){
	    getsym();
	    do{
		vardeclaration();
		while(sym==comma){
		    getsym(); vardeclaration();
		}
		if(sym==semicolon) {
		    getsym();
		}else{
		    error(5);
		}
	    }while(sym==ident);
	}
	while(sym==procsym){
	    getsym();
	    if(sym==ident){
		enter(proc); getsym();
	    }else{
		error(4);
	    }
	    if(sym==semicolon){
		getsym();
	    }else{
		error(5);
	    }
	    lev=lev+1; tx1=tx; dx1=dx;
	    block(fsys|semicolon);
	    lev=lev-1; tx=tx1; dx=dx1;
	    if(sym==semicolon){
		getsym();
		test(statbegsys|ident|procsym,fsys,6);
	    }else{
		error(5);
	    }
	}
	test(statbegsys|ident,declbegsys,7);
    }while(sym&declbegsys);
    code[table[tx0].addr].a=cx;
    table[tx0].addr=cx;		// start addr of code
    cx0=cx; gen(Int,0,dx);
    statement(fsys|semicolon|endsym);
    gen(opr,0,0); // return
    test(fsys,0,8);
    listcode(cx0);
}

long base(long b, long l){
    long b1;

    b1=b;
    while (l>0){	// find base l levels down
	b1=s[b1]; l=l-1;
    }
    return b1;
}

void interpret(){
    long p,b,t;		// program-, base-, topstack-registers
    instruction i;	// instruction register

    printf("start PL/0\n");
    t=0; b=1; p=0;
    s[1]=0; s[2]=0; s[3]=0;
    do{
	i=code[p]; p=p+1;
	switch(i.f){
	    case lit:
		t=t+1; s[t]=i.a;
		break;
	    case opr:
		switch(i.a){ 	// operator
		    case 0:	// return
			t=b-1; p=s[t+3]; b=s[t+2];
			break;
		    case 1:
			s[t]=-s[t];
			break;
		    case 2:
			t=t-1; s[t]=s[t]+s[t+1];
			break;
		    case 3:
			t=t-1; s[t]=s[t]-s[t+1];
			break;
		    case 4:
			t=t-1; s[t]=s[t]*s[t+1];
			break;
		    case 5:
			t=t-1; s[t]=s[t]/s[t+1];
			break;
		    case 6:
			s[t]=s[t]%2;
			break;
		    case 8:
			t=t-1; s[t]=(s[t]==s[t+1]);
			break;
		    case 9:
			t=t-1; s[t]=(s[t]!=s[t+1]);
			break;
		    case 10:
			t=t-1; s[t]=(s[t]<s[t+1]);
			break;
		    case 11:
			t=t-1; s[t]=(s[t]>=s[t+1]);
			break;
		    case 12:
			t=t-1; s[t]=(s[t]>s[t+1]);
			break;
		    case 13:
			t=t-1; s[t]=(s[t]<=s[t+1]);
		}
		break;
	    case lod:
		t=t+1; s[t]=s[base(b,i.l)+i.a];
		break;
	    case sto:
		s[base(b,i.l)+i.a]=s[t]; printf("%10d\n", s[t]); t=t-1;
		break;
	    case cal:		// generate new block mark
		s[t+1]=base(b,i.l); s[t+2]=b; s[t+3]=p;
		b=t+1; p=i.a;
		break;
	    case Int:
		t=t+i.a;
		break;
	    case jmp:
		p=i.a;
		break;
	    case jpc:
		if(s[t]==0){
		    p=i.a;
		}
		t=t-1;
	}
    }while(p!=0);
    printf("end PL/0\n");
}

main(){
	//声明、初始化全局变量
    long i;
    for(i=0; i<256; i++)
	{
		ssym[i]=nul;
    }
	//初始化保留字数组
    strcpy(word[0],  "begin     ");
    strcpy(word[1],  "call      ");
    strcpy(word[2],  "const     ");
    strcpy(word[3],  "do        ");
    strcpy(word[4],  "end       ");
    strcpy(word[5],  "if        ");
    strcpy(word[6],  "odd       ");
    strcpy(word[7],  "procedure ");
    strcpy(word[8],  "then      ");
    strcpy(word[9],  "var       ");
    strcpy(word[10], "while     ");
    wsym[0]=beginsym;
    wsym[1]=callsym;
    wsym[2]=constsym;
    wsym[3]=dosym;
    wsym[4]=endsym;
    wsym[5]=ifsym;
    wsym[6]=oddsym;
    wsym[7]=procsym;
    wsym[8]=thensym;
    wsym[9]=varsym;
    wsym[10]=whilesym;
    ssym['+']=plus;
    ssym['-']=minus;
    ssym['*']=times;
    ssym['/']=slash;
    ssym['(']=lparen;
    ssym[')']=rparen;
    ssym['=']=eql;
    ssym[',']=comma;
    ssym['.']=period;
    ssym[';']=semicolon;
    strcpy(mnemonic[lit],"lit");
    strcpy(mnemonic[opr],"opr");
    strcpy(mnemonic[lod],"lod");
    strcpy(mnemonic[sto],"sto");
    strcpy(mnemonic[cal],"cal");
    strcpy(mnemonic[Int],"int");
    strcpy(mnemonic[jmp],"jmp");
    strcpy(mnemonic[jpc],"jpc");
    declbegsys=constsym|varsym|procsym;
    statbegsys=beginsym|callsym|ifsym|whilesym;
    facbegsys=ident|number|lparen;

	//程序开始，给出提示，读入文件名称，打开文件
    printf("please input source program file name: ");
    scanf("%s",infilename);
    printf("\n");
    if((infile=fopen(infilename,"r"))==NULL)
	{
		printf("File %s can't be opened.\n", infilename);
		exit(1);
    }
    
	//正经的编译器从这里开始
    err=0;
    cc=0; cx=0; ll=0; ch=' '; kk=al; getsym();
    lev=0; tx=0;
    block(declbegsys|statbegsys|period);
    if(sym!=period){
	error(9);
    }
    if(err==0){
	interpret();
    }else{
	printf("errors in PL/0 program\n");
    }
    fclose(infile);
}

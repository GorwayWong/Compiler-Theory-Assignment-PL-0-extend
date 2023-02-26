/*
 * PL/0 complier program for win32 platform (implemented in C)
 *
 * The program has been test on Visual C++ 6.0, Visual C++.NET and
 * Visual C++.NET 2003, on Win98, WinNT, Win2000, WinXP and Win2003
 *
 * 使用方法：
 * 运行后输入PL/0源程序文件?
 * 回答是否输出虚拟机代码
 * 回答是否输出名字表
 * fa.tmp输出虚拟机代码
 * fa1.tmp输出源文件及其各行对应的首地址
 * fa2.tmp输出结?
 * fas.tmp输出名字表
 */

#include <stdio.h>

#include "pl0.h"
#include "string.h"

/* 解释执行时使用的栈 */
#define stacksize 500


int main()
{
	bool nxtlev[symnum];

	printf("Input pl/0 file?   ");
	scanf("%s", fname);     /* 输入文件名 */

	fin = fopen(fname, "r");

	if (fin)
	{
		printf("List object code?(Y/N)");   /* 是否输出虚拟机代码 */
		scanf("%s", fname);
		listswitch = (fname[0]=='y' || fname[0]=='Y');

		printf("List symbol table?(Y/N)");  /* 是否输出名字表 */
		scanf("%s", fname);
		tableswitch = (fname[0]=='y' || fname[0]=='Y');

		fa1 = fopen("fa1.tmp", "w");
		fprintf(fa1,"Input pl/0 file?   ");
		fprintf(fa1,"%s\n",fname);

		init();     /* 初始化 */

		err = 0;
		cc = cx = ll = 0;
		ch = ' ';

		if(-1 != getsym())
		{
			fa = fopen("fa.tmp", "w");
			fas = fopen("fas.tmp", "w");
			addset(nxtlev, declbegsys, statbegsys, symnum);
			nxtlev[period] = true;

			if(-1 == block(0, 0, nxtlev))   /* 调用编译程序 */
			{
				fclose(fa);
				fclose(fa1);
				fclose(fas);
				fclose(fin);
				printf("\n");
				return 0;
			}
			fclose(fa);
			fclose(fa1);
			fclose(fas);

			if (sym != period)
			{
				error(9);
			}

			if (err == 0)
			{
				fa2 = fopen("fa2.tmp", "w");
				interpret();    /* 调用解释执行程序 */
				fclose(fa2);
			}
			else
			{
				printf("Errors in pl/0 program");
			}
		}

		fclose(fin);
	}
	else
	{
		printf("Can't open file!\n");
	}

	printf("\n");
	return 0;
}

/*
* 初始化
*/
void init()
{
	int i;

	/* 设置单字符符号 */
	for (i=0; i<=255; i++)
	{
		ssym[i] = nul;
	}
	ssym['+'] = plus;
	ssym['-'] = minus;
	ssym['*'] = times;
	ssym['/'] = slash;
	ssym['('] = lparen;
	ssym[')'] = rparen;
	ssym['='] = eql;
	ssym[','] = comma;
	ssym['.'] = period;
	ssym['#'] = neq;
	ssym[';'] = semicolon;
	ssym['%'] = formatsym;

	/* 设置保留字名字,按照字母顺序，便于折半查找 */
	strcpy(&(word[0][0]), "avg");//extend
	strcpy(&(word[1][0]), "begin");
	strcpy(&(word[2][0]), "call");
	strcpy(&(word[3][0]), "ceil");//extend
	strcpy(&(word[4][0]), "const");
	strcpy(&(word[5][0]), "do");
	strcpy(&(word[6][0]), "end");
	strcpy(&(word[7][0]),"floating");//extend
	strcpy(&(word[8][0]),"floatlist");//extend
	strcpy(&(word[9][0]),"floor");//extend
	strcpy(&(word[10][0]), "if");
	strcpy(&(word[11][0]), "intlist");//extend
	strcpy(&(word[12][0]), "max");//extend
	strcpy(&(word[13][0]), "min");//extend
	strcpy(&(word[14][0]), "odd");
	strcpy(&(word[15][0]), "printf");//extend
	strcpy(&(word[16][0]), "procedure");
	strcpy(&(word[17][0]), "read");
	strcpy(&(word[18][0]), "scanf");//extend
	strcpy(&(word[19][0]), "sum");//extend sum()
	strcpy(&(word[20][0]), "then");
	strcpy(&(word[21][0]), "var");
	strcpy(&(word[22][0]), "while");
	strcpy(&(word[23][0]), "write");

	/* 设置保留字符号 */
	wsym[0] = avgsym;//extend
	wsym[1] = beginsym;
	wsym[2] = callsym;
	wsym[3] = ceilsym;//extend 向上取整
	wsym[4] = constsym;
	wsym[5] = dosym;
	wsym[6] = endsym;
	wsym[7] = floatsym;//extend 浮点数
	wsym[8] = floatarrsym;//extend 浮点数组
	wsym[9] = floorsym;//extend 向下取整
 	wsym[10] = ifsym;
	wsym[11] = intarrsym;//extend 整数数组
	wsym[12] = maxsym;//extend 整数数组
	wsym[13] = minsym;//extend 整数数组
	wsym[14] = oddsym;
	wsym[15] = printfsym;//extend
	wsym[16] = procsym;
	wsym[17] = readsym;
	wsym[18] = scanfsym;//extend
	wsym[19] = sumsym;//extend
	wsym[20] = thensym;
	wsym[21] = varsym;
	wsym[22] = whilesym;
	wsym[23] = writesym;

	/* 设置指令名称 */
	strcpy(&(mnemonic[lit][0]), "lit");
	strcpy(&(mnemonic[opr][0]), "opr");
	strcpy(&(mnemonic[lod][0]), "lod");
	strcpy(&(mnemonic[sto][0]), "sto");
	strcpy(&(mnemonic[cal][0]), "cal");
	strcpy(&(mnemonic[inte][0]), "int");
	strcpy(&(mnemonic[jmp][0]), "jmp");
	strcpy(&(mnemonic[jpc][0]), "jpc");

	/* 设置符号集 */
	for (i=0; i<symnum; i++)
	{
		declbegsys[i] = false;
		statbegsys[i] = false;
		facbegsys[i] = false;
	}

	/* 设置声明开始符号集 */
	declbegsys[constsym] = true;
	declbegsys[varsym] = true;
	declbegsys[procsym] = true;
	declbegsys[intarrsym] = true;//extend 整型数组
	declbegsys[floatarrsym] = true;//extend 浮点数组

	/* 设置语句开始符号集 */
	statbegsys[beginsym] = true;
	statbegsys[callsym] = true;
	statbegsys[ifsym] = true;
	statbegsys[whilesym] = true;
	statbegsys[readsym] = true;
	statbegsys[writesym] = true;
	statbegsys[printfsym] = true;
	statbegsys[scanfsym] = true;

	/* 设置因子开始符号集 */
	facbegsys[ident] = true;
	facbegsys[number] = true;
	facbegsys[lparen] = true;
	facbegsys[floatsym] = true;
	facbegsys[floorsym] = true;
	facbegsys[ceilsym] = true;
	facbegsys[sumsym] = true;
	facbegsys[avgsym] = true;
	facbegsys[maxsym] = true;
	facbegsys[minsym] = true;
}

/*
* 用数组实现集合的集合运算
*/
int inset(int e, bool* s)
{
	return s[e];
}

int addset(bool* sr, bool* s1, bool* s2, int n)
{
	int i;
	for (i=0; i<n; i++)
	{
		sr[i] = s1[i]||s2[i];
	}
	return 0;
}

int subset(bool* sr, bool* s1, bool* s2, int n)
{
	int i;
	for (i=0; i<n; i++)
	{
		sr[i] = s1[i]&&(!s2[i]);
	}
	return 0;
}

int mulset(bool* sr, bool* s1, bool* s2, int n)
{
	int i;
	for (i=0; i<n; i++)
	{
		sr[i] = s1[i]&&s2[i];
	}
	return 0;
}

/*
*   出错处理，打印出错位置和错误编码
*/
void error(int n)
{
	char space[81];
	memset(space,32,81);

	space[cc-1]=0; //出错时当前符号已经读完，所以cc-1

	printf("****%s!%d\n", space, n);
	fprintf(fa1,"****%s!%d\n", space, n);
	switch (n)
	{
	case 22:
		printf("缺少右括号！\n");
		break;
	case 30:
		printf("数字过长！\n");
		break;
	case 5:
		printf("漏掉了英文逗号或者是英文分号！\n");
		break;
	case 9:
		printf("程序末尾漏掉了英文句号!");
		break;
	case 32:
		printf("嵌套声明太多！\n");
		break;
	default:
		printf("未知错误！\n");
		break;
	}

	err++;
}

/*
* 漏掉空格，读取一个字符。
*
* 每次读一行，存入line缓冲区，line被getsym取空后再读一行
*
* 被函数getsym调用。
*/
int getch()
{
	if (cc == ll)
	{
		if (feof(fin))
		{
			printf("program incomplete");
			return -1;
		}
		ll=0;
		cc=0;
		printf("%d ", cx);
		fprintf(fa1,"%d ", cx);
		ch = ' ';
		while (ch != 10)
		{
			//fscanf(fin,"%c", &ch)
			//richard
			if (EOF == fscanf(fin,"%c", &ch))
			{
				line[ll] = 0;
				break;
			}
			//end richard
			printf("%c", ch);
			fprintf(fa1, "%c", ch);
			line[ll] = ch;
			ll++;
		}
		printf("\n");
		fprintf(fa1, "\n");
	}
	ch = line[cc];
	cc++;
	return 0;
}

/*
* 词法分析，获取一个符号
*/
int getsym()
{
	int i,j,k;

	/* the original version lacks "\r", thanks to foolevery */
	while (ch==' ' || ch==10 || ch==13 || ch==9)  /* 忽略空格、换行、回车和TAB */
	{
		getchdo;
	}
	if (ch>='a' && ch<='z')
	{           /* 名字或保留字以a..z开头 */
		k = 0;
		do {
			if(k<al)
			{
				a[k] = ch;
				k++;
			}
			getchdo;
		} while (ch>='a' && ch<='z' || ch>='0' && ch<='9');
		a[k] = 0;
		strcpy(id, a);
		i = 0;
		j = norw-1;
		do {    /* 搜索当前符号是否为保留字 */
			k = (i+j)/2;
			if (strcmp(id,word[k]) <= 0)
			{
				j = k - 1;
			}
			if (strcmp(id,word[k]) >= 0)
			{
				i = k + 1;
			}
		} while (i <= j);
		if (i-1 > j)
		{
			sym = wsym[k];
		}
		else
		{
			sym = ident; /* 搜索失败则，是名字或数字 */
		}
	}
	else
	{
		if (ch>='0' && ch<='9')
		{           /* 检测是否为数字：以0..9开头 */
			k = 0;
			num = 0;
			sym = number;
			do {
				num = 10*num + ch - '0';
				k++;
				getchdo;
			} while (ch>='0' && ch<='9'); /* 获取数字的值 */
			if(ch=='.'){//判断是否是浮点 处理小数部分 extend
				sym = floatsym;
				getchdo;
				float temp = 0.1;
				floatNum = (float)num;
				while (ch>='0'&&ch<='9')
				{
					floatNum+=temp*(ch-'0');
					temp/=10.0;
					k++;
					getchdo;
				}
			}
			k--;
			if (k > nmax)
			{
				error(30);
			}
		}
		else
		{
			if (ch == ':')      /* 检测赋值符号 */
			{
				getchdo;
				if (ch == '=')
				{
					sym = becomes;
					getchdo;
				}
				else
				{
					sym = nul;  /* 不能识别的符号 */
				}
			}
			else
			{
				if (ch == '<')      /* 检测小于或小于等于符号 */
				{
					getchdo;
					if (ch == '=')
					{
						sym = leq;
						getchdo;
					}
					else
					{
						sym = lss;
					}
				}
				else
				{
					if (ch=='>')        /* 检测大于或大于等于符号 */
					{
						getchdo;
						if (ch == '=')
						{
							sym = geq;
							getchdo;
						}
						else
						{
							sym = gtr;
						}
					}
					else
					{
						sym = ssym[ch];     /* 当符号不满足上述条件时，全部按照单字符符号处理 */
						//getchdo;
						//richard
						if (sym != period)
						{
							getchdo;
						}
						//end richard
					}
				}
			}
		}
	}
	return 0;
}

/*
* 生成虚拟机代码
*
* x: instruction.f;
* y: instruction.l;
* z: instruction.a;
*/
int gen(enum fct x, int y, int z ,float isf)
{
	if (cx >= cxmax)
	{
		printf("Program too long"); /* 程序过长 */
		return -1;
	}
	code[cx].f = x;
	code[cx].l = y;
	code[cx].a = z;
	code[cx].isFlow = isf;
	cx++;
	return 0;
}


/*
* 测试当前符号是否合法
*
* 在某一部分（如一条语句，一个表达式）将要结束时时我们希望下一个符号属于某集?
* （该部分的后跟符号），test负责这项检测，并且负责当检测不通过时的补救措施，
* 程序在需要检测时指定当前需要的符号集合和补救用的集合（如之前未完成部分的后跟
* 符号），以及检测不通过时的错误号。
*
* s1:   我们需要的符号
* s2:   如果不是我们需要的，则需要一个补救用的集?
* n:    错误号
*/
int test(bool* s1, bool* s2, int n)
{
	if (!inset(sym, s1))
	{
		error(n);
		/* 当检测不通过时，不停获取符号，直到它属于需要的集合或补救的集合 */
		while ((!inset(sym,s1)) && (!inset(sym,s2)))
		{
			getsymdo;
		}
	}
	return 0;
}

/*
* 编译程序主?
*
* lev:    当前分程序所在层
* tx:     名字表当前尾指针
* fsys:   当前模块后跟符号集?
*/
int block(int lev, int tx, bool* fsys)
{
	int i;

	int dx;                 /* 名字分配到的相对地址 */
	int tx0;                /* 保留初始tx */
	int cx0;                /* 保留初始cx */
	bool nxtlev[symnum];    /* 在下级函数的参数中，符号集合均为值参，但由于使用数组实现，
							传递进来的是指针，为防止下级函数改变上级函数的集合，开辟新的空?
							传递给下级函数*/

	dx = 3;
	tx0 = tx;               /* 记录本层名字的初始位置 */
	table[tx].adr = cx;

	gendo(jmp, 0, 0,0);

	if (lev > levmax)
	{
		error(32);
	}

	do {

		if (sym == constsym)    /* 收到常量声明符号，开始处理常量声明 */
		{
			getsymdo;

			/* the original do...while(sym == ident) is problematic, thanks to calculous */
			/* do { */
			constdeclarationdo(&tx, lev, &dx);  /* dx的值会被constdeclaration改变，使用指针 */
			while (sym == comma)
			{
				getsymdo;
				constdeclarationdo(&tx, lev, &dx);
			}
			if (sym == semicolon)
			{
				getsymdo;
			}
			else
			{
				error(5);   /*漏掉了逗号或者分号*/
			}
			/* } while (sym == ident); */
		}

		if (sym == varsym)      /* 收到变量声明符号，开始处理变量声明 */
		{
			getsymdo;
			int tx_cp = tx+1;
			int forloop;

			/* the original do...while(sym == ident) is problematic, thanks to calculous */
			/* do {  */
			vardeclarationdo(&tx, lev, &dx);
			while (sym == comma)
			{
				getsymdo;
				vardeclarationdo(&tx, lev, &dx);
			}
			for(forloop =tx_cp;forloop<=tx;forloop++){
				table[forloop].type = 0;
			}
			if (sym == semicolon)
			{
				getsymdo;
			}
			else
			{
				error(5);
			}
			/* } while (sym == ident);  */
		}
		if (sym==floatsym){//extend 浮点检测
			//printf("float!\n");
			getsymdo;			
			int tx_cp = tx+1;
			int forloop;

			vardeclarationdo(&tx, lev, &dx);
			while (sym == comma)
			{
				getsymdo;
				vardeclarationdo(&tx, lev, &dx);
			}
			for(forloop =tx_cp;forloop<=tx;forloop++){
				table[forloop].type = 1;
			}
			if (sym == semicolon)
			{
				getsymdo;
			}
			else
			{
				error(5);
			}
		}
		if(sym == intarrsym){//extend 整数数组声明 一次只能声明一个数组
			getsymdo;
			intarrdeclarationdo(&tx,lev,&dx);
			if(sym==semicolon){
				getsymdo;
			}
			else{
				error(5);
			}
		}
		if(sym == floatarrsym){//extend 浮点数组声明 一次只能声明一个数组
			getsymdo;
			floatarrdeclarationdo(&tx,lev,&dx);
			if(sym==semicolon){
				getsymdo;
			}
			else{
				error(5);
			}
		}

		while (sym == procsym) /* 收到过程声明符号，开始处理过程声明 */
		{
			getsymdo;

			if (sym == ident)
			{
				enter(procedur, &tx, lev, &dx); /* 记录过程名字 */
				getsymdo;
			}
			else
			{
				error(4);   /* procedure后应为标识符 */
			}

			if (sym == semicolon)
			{
				getsymdo;
			}
			else
			{
				error(5);   /* 漏掉了分号 */
			}

			memcpy(nxtlev, fsys, sizeof(bool)*symnum);
			nxtlev[semicolon] = true;
			if (-1 == block(lev+1, tx, nxtlev))
			{
				return -1;  /* 递归调用 */
			}

			if(sym == semicolon)
			{
				getsymdo;
				memcpy(nxtlev, statbegsys, sizeof(bool)*symnum);
				nxtlev[ident] = true;
				nxtlev[procsym] = true;
				testdo(nxtlev, fsys, 6);
			}
			else
			{
				error(5);   /* 漏掉了分号 */
			}
		}
		memcpy(nxtlev, statbegsys, sizeof(bool)*symnum);
		nxtlev[ident] = true;
		testdo(nxtlev, declbegsys, 7);
	} while (inset(sym, declbegsys));   /* 直到没有声明符号 */

	code[table[tx0].adr].a = cx;    /* 开始生成当前过程代码 */
	table[tx0].adr = cx;            /* 当前过程代码地址 */
	table[tx0].size = dx;           /* 声明部分中每增加一条声明都会给dx增加1，声明部分已经结束，dx就是当前过程数据的size */
	cx0 = cx;
	gendo(inte, 0, dx,0);             /* 生成分配内存代码 */

	if (tableswitch)        /* 输出名字表 */
	{
		printf("TABLE:\n");
		if (tx0+1 > tx)
		{
			printf("    NULL\n");
		}
		for (i=tx0+1; i<=tx; i++)
		{
			switch (table[i].kind)
			{
			case constant:
				printf("    %d const %s ", i, table[i].name);
				printf("val=%d\n", table[i].val);
				fprintf(fas, "    %d const %s ", i, table[i].name);
				fprintf(fas, "val=%d\n", table[i].val);
				break;
			case variable:
				printf("    %d var   %s ", i, table[i].name);
				printf("lev=%d addr=%d\n", table[i].level, table[i].adr);
				fprintf(fas, "    %d var   %s ", i, table[i].name);
				fprintf(fas, "lev=%d addr=%d\n", table[i].level, table[i].adr);
				break;
			case procedur:
				printf("    %d proc  %s ", i, table[i].name);
				printf("lev=%d addr=%d size=%d\n", table[i].level, table[i].adr, table[i].size);
				fprintf(fas,"    %d proc  %s ", i, table[i].name);
				fprintf(fas,"lev=%d addr=%d size=%d\n", table[i].level, table[i].adr, table[i].size);
				break;
			}
		}
		printf("\n");
	}

	/* 语句后跟符号为分号或end */
	memcpy(nxtlev, fsys, sizeof(bool)*symnum);  /* 每个后跟符号集和都包含上层后跟符号集和，以便补救 */
	nxtlev[semicolon] = true;
	nxtlev[endsym] = true;
	statementdo(nxtlev, &tx, lev);
	gendo(opr, 0, 0,0);                       /* 每个过程出口都要使用的释放数据段指令 */
	memset(nxtlev, 0, sizeof(bool)*symnum); /*分程序没有补救集合 */
	testdo(fsys, nxtlev, 8);                /* 检测后跟符号正确性 */
	listcode(cx0);                          /* 输出代码 */
	return 0;
}

/*
* 在名字表中加入一项
*
* k:      名字种类const,var or procedure
* ptx:    名字表尾指针的指针，为了可以改变名字表尾指针的值
* lev:    名字所在的层次,，以后所有的lev都是这样
* pdx:    dx为当前应分配的变量的相对地址，分配后要增加1
*/
void enter(enum object k, int* ptx, int lev, int* pdx)
{
	(*ptx)++;
	strcpy(table[(*ptx)].name, id); /* 全局变量id中已存有当前名字的名字 */
	table[(*ptx)].kind = k;
	switch (k)
	{
	case constant:  /* 常量名字 */
		if (num > amax)
		{
			error(31);  /* 数越界 */
			num = 0;
		}
		if(table[(*ptx)].type==1){
			table[(*ptx)].valf = floatNum;
		}else{
			table[(*ptx)].val = num;
		}
		break;
	case variable:  /* 变量名字 */
		table[(*ptx)].level = lev;
		table[(*ptx)].adr = (*pdx);
		(*pdx)++;
		break;
	case procedur:  /*　过程名字　*/
		table[(*ptx)].level = lev;
		break;
	case intarray:
		table[(*ptx)].level = lev;
		table[(*ptx)].adr=(*pdx);
		table[(*ptx)].size=num;
		(*pdx) = (*pdx) + num;
		break;
	case floatarray:
		table[(*ptx)].level = lev;
		table[(*ptx)].adr=(*pdx);
		table[(*ptx)].size=(int)num;
		(*pdx) = (*pdx) + (int)num;
	}
}

/*
* 查找名字的位置.
* 找到则返回在名字表中的位置,否则返回0.
*
* idt:    要查找的名字
* tx:     当前名字表尾指针
*/
int position(char* idt, int tx)
{
	int i;
	strcpy(table[0].name, idt);
	i = tx;
	while (strcmp(table[i].name, idt) != 0)
	{
		i--;
	}
	return i;
}

/*
* 常量声明处理
*/
int constdeclaration(int* ptx, int lev, int* pdx)
{
	if (sym == ident)
	{
		getsymdo;
		if (sym==eql || sym==becomes)
		{
			if (sym == becomes)
			{
				error(1);   /* 把=写成了:= */
			}
			getsymdo;
			if (sym == number || sym == floatsym)
			{
				if(sym==number){
                    table[(*ptx+1)].type = 0;
                }else if(sym==floatsym){
                    table[(*ptx+1)].type = 1;
                }
				enter(constant, ptx, lev, pdx);
				getsymdo;
			}
			else
			{
				error(2);   /* 常量说明=后应是数字 */
			}
		}
		else
		{
			error(3);   /* 常量说明标识后应是= */
		}
	}
	else
	{
		error(4);   /* const后应是标识 */
	}
	return 0;
}

/*
* 变量声明处理
*/
int vardeclaration(int* ptx,int lev,int* pdx)
{
	if (sym == ident)
	{
		enter(variable, ptx, lev, pdx); // 填写名字表
		getsymdo;
	}
	else
	{
		error(4);   /* var后应是标识 */
	}
	return 0;
}

int intarrdeclaration(int* ptx,int lev,int *pdx){//extend
	if (sym == ident)
	{
		getsymdo;
		if(sym == lparen){//左括号
			getsymdo;
			if(sym == number){
				if(num>0){
					enter(intarray, ptx, lev, pdx);
					getsymdo;
					if(sym == rparen){
						getsymdo;
					}else{
						error(101);//数组声明缺少右括号 
					}
				}else{
					error(102);//数组大小输入错误 
				}
			}
			else{
				error(103);//请输入数组大小
			}
		}
		else{
			error(48);//数组声明缺少左括号 
		}
		
	}
	else
	{
		error(4);   /* var后应是标识 */
	}
	return 0;
}

int floatarrdeclaration(int* ptx,int lev,int *pdx){//extend
	if (sym == ident)
	{
		getsymdo;
		if(sym == lparen){//左括号
			getsymdo;
			if(sym == number){
				if(num>0){
					enter(floatarray, ptx, lev, pdx);
					getsymdo;
					if(sym == rparen){
						getsymdo;
					}else{
						error(101);//数组声明缺少右括号 
					}
				}else{
					error(102);//数组大小输入错误 
				}
			}
			else{
				error(103);//请输入数组大小
			}
		}
		else{
			error(48);//数组声明缺少左括号 
		}
		
	}
	else
	{
		error(4);   /* var后应是标识 */
	}
	return 0;
}


/*
* 输出目标代码清单
*/
void listcode(int cx0)
{
	int i;
	if (listswitch)
	{
		for (i=cx0; i<cx; i++)
		{
			printf("%d %s %d %d\n", i, mnemonic[code[i].f], code[i].l, code[i].a);
			fprintf(fa,"%d %s %d %d\n", i, mnemonic[code[i].f], code[i].l, code[i].a);
		}
	}
}

/*
* 语句处理
*/
int statement(bool* fsys, int* ptx, int lev)
{
	int i, cx1, cx2;
	bool nxtlev[symnum];

	if (sym == ident)   /* 准备按照赋值语句处理 */
	{
		i = position(id, *ptx);
		if (i == 0)
		{
			error(11);  /* 变量未找到 */
		}
		else
		{
			if(table[i].kind != variable)//说明可能是数组
			{
				if(table[i].kind == intarray){//extend 整型数组
					getsymdo;
					if(sym == lparen){
						getsymdo;
						if(sym == number){
							getsymdo;
							int offset=num;
							if(sym == rparen){
								getsymdo;
								if(sym == becomes)
								{
									getsymdo;
								}
								else
								{
									error(13);  /* 没有检测到赋值符号 */
								}
								memcpy(nxtlev, fsys, sizeof(bool)*symnum);
								expressiondo(nxtlev, ptx, lev); /* 处理赋值符号右侧表达式 */
								if(i != 0)
								{
									/* expression将执行一系列指令，但最终结果将会保存在栈顶，执行sto命令完成赋值 */
									gendo(sto, lev-table[i].level, table[i].adr+offset,0);
								}
							}
						}
					}
					else{
						error(104);//数组下标错误
					}
				}else if(table[i].kind == floatarray){
					getsymdo;
					if(sym == lparen){
						getsymdo;
						if(sym == number){
							getsymdo;
							int offset=num;
							if(sym == rparen){
								getsymdo;
								if(sym == becomes)
								{
									getsymdo;
								}
								else
								{
									error(13);  /* 没有检测到赋值符号 */
								}
								memcpy(nxtlev, fsys, sizeof(bool)*symnum);
								expressiondo(nxtlev, ptx, lev); /* 处理赋值符号右侧表达式 */
								if(i != 0)
								{
									/* expression将执行一系列指令，但最终结果将会保存在栈顶，执行sto命令完成赋值 */
									gendo(sto, lev-table[i].level, table[i].adr+offset,1);
								}
							}
						}
					}
					else{
						error(104);//数组下标错误
					}
				}else{
					error(12);  /* 赋值语句格式错误 */
					i = 0;
				}
			}
			else
			{
				getsymdo;
				if(sym == becomes)
				{
					getsymdo;
				}
				else
				{
					error(13);  /* 没有检测到赋值符号 */
				}
				memcpy(nxtlev, fsys, sizeof(bool)*symnum);
				expressiondo(nxtlev, ptx, lev); /* 处理赋值符号右侧表达式 */
				if(i != 0)
				{
				if(table[i].type==0){
                    // 按整型存入变量
                    gendo(sto, lev-table[i].level, table[i].adr, 0);
                }
                else if(table[i].type==1){
                	// 按浮点数存入变量
                    gendo(sto, lev-table[i].level, table[i].adr, 1);
                }else{
                    printf("存入变量值出错！\n");
                }
					// /* expression将执行一系列指令，但最终结果将会保存在栈顶，执行sto命令完成赋值 */
					// gendo(sto, lev-table[i].level, table[i].adr);
				}
			}
		}//if (i == 0)
	}
	else
	{
		if (sym == readsym) /* 准备按照read语句处理 */
		{
			getsymdo;
			if (sym != lparen)
			{
				error(34);  /* 格式错误，应是左括号 */
			}
			else
			{
				do {
					getsymdo;
					if (sym == ident)
					{
						i = position(id, *ptx); /* 查找要读的变量 */
					}
					else
					{
						i=0;
					}

					if (i == 0)
					{
						error(35);  /* read()中应是声明过的变量名 */
					}
					else if (table[i].kind != variable)
					{
						error(32);	/* read()参数表的标识符不是变量, thanks to amd */
					}
					else
					{
						gendo(opr, 0, 16,0);  /* 生成输入指令，读取值到栈顶 */
						gendo(sto, lev-table[i].level, table[i].adr,0);   /* 储存到变量 */
					}
					getsymdo;

				} while (sym == comma); /* 一条read语句可读多个变量 */
			}
			if(sym != rparen)
			{
				printf("here\n");
				error(33);  /* 格式错误，应是右括号 */
				while (!inset(sym, fsys))   /* 出错补救，直到收到上层函数的后跟符号 */
				{
					getsymdo;
				}
			}
			else
			{
				getsymdo;
			}
		}
		else if(sym==scanfsym){
			getsymdo;
			if(sym!=lparen){
				error(34);
			}else{
				do{
					getsymdo;
					if(sym==formatsym){
						if(ch=='d'){//整型
							getchdo;
							getsymdo;
							if(sym==comma){
								getsymdo;
								if(sym==ident){
									i = position(id, *ptx); /* 查找要读的变量 */
								}
								else{
									i=0;
								}
								if(i==0){
									error(35);
								}else if(table[i].kind!=variable&&table[i].kind!=intarray){
									error(32);//除了整型变量 数组之外不能输入
								}else{
									if(table[i].kind==variable){
										//printf("赋值\n");
										gendo(opr,0,17,0);
										gendo(sto, lev-table[i].level, table[i].adr,0);   /* 储存到变量 */
									}else if(table[i].kind==intarray){
										getsymdo;
										if(sym==lparen){
											getsymdo;
											if(sym==number){
												getsymdo;
												if(sym!=rparen){
													error(51);
												}else{
													int offset=num;
													gendo(opr, 0, 17,0);  /* 生成输入指令，读取值到栈顶 */
													gendo(sto, lev-table[i].level, table[i].adr+offset,0);   /* 储存到变量 */
												}
											}
											else{
												error(50);
											}
										}else{
											error(49);
										}
									}
								}
								getsymdo;
							}
						}else if(ch=='f'){
							getchdo;
							getsymdo;
							if(sym==comma){
								getsymdo;
								if(sym==ident){
									i = position(id, *ptx); /* 查找要读的变量 */
								}
								else{
									i=0;
								}

								if(i==0){
									error(35);
								}
								else if (table[i].kind != variable&&table[i].kind != floatarray)
								{
									error(32);	/* read()参数表的标识符不是变量, thanks to amd */
								}else{
									if(table[i].kind==variable){
										gendo(opr,0,16,0);
										gendo(sto, lev-table[i].level, table[i].adr,1);   /* 储存到变量 */
									}
									else if(table[i].kind == floatarray){
										getsymdo;
										if(sym==lparen){
											getsymdo;
											if(sym==number){
												getsymdo;
												int offset = num;
												gendo(opr, 0, 16,0);  /* 生成输入指令，读取值到栈顶 */
												gendo(sto, lev-table[i].level, table[i].adr+offset,1);   /* 储存到变量 */
												if(sym != rparen){
													error(51);//数组调用缺少右括号 
												}
											}
											else{
												error(50);
											}
										}
										else{
											error(49);
										}
									}
								}
								getsymdo;
							}
						}
					}else{
						error(42);
					}
				}while(sym==comma);
			}
			if(sym!=rparen){
				error(33);
				while(!inset(sym,fsys)){
					getsymdo;
				}
			}else{
				getsymdo;
			}
		}
		else
		{
			if (sym == writesym)    /* 准备按照write语句处理，与read类似 */
			{
				getsymdo;
				if (sym == lparen)
				{
					do {
						getsymdo;
						memcpy(nxtlev, fsys, sizeof(bool)*symnum);
						nxtlev[rparen] = true;
						//printf("stuck!\n");
						nxtlev[comma] = true;       /* write的后跟符号为) or , */
						expressiondo(nxtlev, ptx, lev); /* 调用表达式处理，此处与read不同，read为给变量赋值 */
						gendo(opr, 0, 14,0);  /* 生成输出指令，输出栈顶的值 */
					} while (sym == comma);
					if (sym != rparen)
					{
						error(33);  /* write()中应为完整表达式 */
					}
					else
					{
						getsymdo;
					}
				}
				gendo(opr, 0, 15,0);  /* 输出换行 */
			}
			else if(sym == printfsym){
				getsymdo;
				if(sym==lparen){
					do{
						getsymdo;
						if(sym==formatsym){
							if(ch=='d'||ch=='f'){
								getchdo;
								getsymdo;
								if(sym==comma){
									getsymdo;
									memcpy(nxtlev, fsys, sizeof(bool)*symnum);
									nxtlev[rparen] = true;
									nxtlev[comma] = true;       /* write的后跟符号为) or , */
									expressiondo(nxtlev, ptx, lev); /* 调用表达式处理，此处与read不同，read为给变量赋值 */
									gendo(opr, 0, 14,0);  /* 生成整数输出指令，输出栈顶的值 */
								}
							}
						}
					}while(sym==comma);
					if(sym!=rparen){
						error(33);
					}
					else{
						getsymdo;
					}
				}
				gendo(opr, 0, 15,0);  /* 输出换行 */
			}
			else
			{
				if (sym == callsym) /* 准备按照call语句处理 */
				{
					getsymdo;
					if (sym != ident)
					{
						error(14);  /* call后应为标识符 */
					}
					else
					{
						i = position(id, *ptx);
						if (i == 0)
						{
							error(11);  /* 过程未找到 */
						}
						else
						{
							if (table[i].kind == procedur)
							{
								gendo(cal, lev-table[i].level, table[i].adr,0);   /* 生成call指令 */
							}
							else
							{
								error(15);  /* call后标识符应为过程 */
							}
						}
						getsymdo;
					}
				}
				else
				{
					if (sym == ifsym)   /* 准备按照if语句处理 */
					{
						getsymdo;
						memcpy(nxtlev, fsys, sizeof(bool)*symnum);
						nxtlev[thensym] = true;
						nxtlev[dosym] = true;   /* 后跟符号为then或do */
						conditiondo(nxtlev, ptx, lev); /* 调用条件处理（逻辑运算）函数 */
						if (sym == thensym)
						{
							getsymdo;
						}
						else
						{
							error(16);  /* 缺少then */
						}
						cx1 = cx;   /* 保存当前指令地址 */
						gendo(jpc, 0, 0,0);   /* 生成条件跳转指令，跳转地址未知，暂时写0 */
						statementdo(fsys, ptx, lev);    /* 处理then后的语句 */
						code[cx1].a = cx;   /* 经statement处理后，cx为then后语句执行完的位置，它正是前面未定的跳转地址 */
					}
					else
					{
						if (sym == beginsym)    /* 准备按照复合语句处理 */
						{
							getsymdo;
							memcpy(nxtlev, fsys, sizeof(bool)*symnum);
							nxtlev[semicolon] = true;
							nxtlev[endsym] = true;  /* 后跟符号为分号或end */
							/* 循环调用语句处理函数，直到下一个符号不是语句开始符号或收到end */
							statementdo(nxtlev, ptx, lev);

							while (inset(sym, statbegsys) || sym==semicolon)
							{
								if (sym == semicolon)
								{
									getsymdo;
								}
								else
								{
									error(10);  /* 缺少分号 */
								}
								statementdo(nxtlev, ptx, lev);
							}
							if(sym == endsym)
							{
								getsymdo;
							}
							else
							{
								error(17);  /* 缺少end或分号 */
							}
						}
						else
						{
							if (sym == whilesym)    /* 准备按照while语句处理 */
							{
								cx1 = cx;   /* 保存判断条件操作的位置 */
								getsymdo;
								memcpy(nxtlev, fsys, sizeof(bool)*symnum);
								nxtlev[dosym] = true;   /* 后跟符号为do */
								conditiondo(nxtlev, ptx, lev);  /* 调用条件处理 */
								cx2 = cx;   /* 保存循环体的结束的下一个位置 */
								gendo(jpc, 0, 0,0);   /* 生成条件跳转，但跳出循环的地址未知 */
								if (sym == dosym)
								{
									getsymdo;
								}
								else
								{
									error(18);  /* 缺少do */
								}
								statementdo(fsys, ptx, lev);    /* 循环体 */
								gendo(jmp, 0, cx1,0); /* 回头重新判断条件 */
								code[cx2].a = cx;   /* 反填跳出循环的地址，与if类似 */
							}
							else
							{
								memset(nxtlev, 0, sizeof(bool)*symnum); /* 语句结束无补救集合 */
								testdo(fsys, nxtlev, 19);   /* 检测语句结束的正确性 */
							}
						}
					}
				}
			}
		}
	}
	return 0;
}

/*
* 表达式处理
*/
int expression(bool* fsys, int* ptx, int lev)
{
	enum symbol addop;  /* 用于保存正负号 */
	bool nxtlev[symnum];

	if(sym==plus || sym==minus) /* 开头的正负号，此时当前表达式被看作一个正的或负的项 */
	{
		addop = sym;    /* 保存开头的正负号 */
		getsymdo;
		memcpy(nxtlev, fsys, sizeof(bool)*symnum);
		nxtlev[plus] = true;
		nxtlev[minus] = true;
		termdo(nxtlev, ptx, lev);   /* 处理项 */
		if (addop == minus)
		{
			gendo(opr,0,1,0); /* 如果开头为负号生成取负指令 */
		}
	}
	else    /* 此时表达式被看作项的加减 */
	{
		memcpy(nxtlev, fsys, sizeof(bool)*symnum);
		nxtlev[plus] = true;
		nxtlev[minus] = true;
		termdo(nxtlev, ptx, lev);   /* 处理项 */
	}
	while (sym==plus || sym==minus)
	{
		addop = sym;
		getsymdo;
		memcpy(nxtlev, fsys, sizeof(bool)*symnum);
		nxtlev[plus] = true;
		nxtlev[minus] = true;
		termdo(nxtlev, ptx, lev);   /* 处理项 */
		if (addop == plus)
		{
			gendo(opr, 0, 2,0);   /* 生成加法指令 */
		}
		else
		{
			gendo(opr, 0, 3,0);   /* 生成减法指令 */
		}
	}
	return 0;
}

/*
* 项处理
*/
int term(bool* fsys, int* ptx, int lev)
{
	enum symbol mulop;  /* 用于保存乘除法符号 */
	bool nxtlev[symnum];

	memcpy(nxtlev, fsys, sizeof(bool)*symnum);
	nxtlev[times] = true;
	nxtlev[slash] = true;
	factordo(nxtlev, ptx, lev); /* 处理因子 */
	while(sym==times || sym==slash)
	{
		mulop = sym;
		getsymdo;
		factordo(nxtlev, ptx, lev);
		if(mulop == times)
		{
			gendo(opr, 0, 4,0);   /* 生成乘法指令 */
		}
		else
		{
			gendo(opr, 0, 5,0);   /* 生成除法指令 */
		}
	}
	return 0;
}

/*
* 因子处理
*/
int factor(bool* fsys, int* ptx, int lev)
{
	int i;
	bool nxtlev[symnum];
	testdo(facbegsys, fsys, 24);    /* 检测因子的开始符号 */
	/* while(inset(sym, facbegsys)) */  /* 循环直到不是因子开始符号 */
	if(inset(sym,facbegsys))    /* BUG: 原来的方法var1(var2+var3)会被错误识别为因子 */
	{
		if(sym == ident)    /* 因子为常量或变量 */
		{
			i = position(id, *ptx); /* 查找名字 */
			if (i == 0)
			{
				error(11);  /* 标识符未声明 */
			}
			else
			{
				switch (table[i].kind)
				{
				case constant:  /* 名字为常量 */

					if (table[i].type==0){
                        gendo(lit, 0, table[i].val,0);    /* 直接把常量的值入栈 */
                    }
                    else if(table[i].type==1){
                        gendo(lit, 1, table[i].valf,1);    /* 直接把常量的值入栈 */
                    }
					break;
				case variable:  /* 名字为变量 */
					if (table[i].type==0){//extend
						gendo(lod, lev-table[i].level, table[i].adr,0);   /* 找到变量地址并将其值入栈 */
					}else{
						//printf("浮点入栈！\n");
						gendo(lod, lev-table[i].level, table[i].adr,1);   /* 找到变量地址并将其值入栈 */
					}
					break;
				case procedur:  /* 名字为过程 */
					error(21);  /* 不能为过程 */
					break;
				case intarray://extend 整型数组处理
					getsymdo;
					if(sym == lparen){
						getsymdo;
						if(sym == number){
							int offset=num;
							getsymdo;
							gendo(lod, lev-table[i].level, table[i].adr+offset,0);
							if(sym != rparen){
								error(51);
							}
						}
						else{
							error(50);
						} 
						 
					}
					else{
						error(49);	
					}
					break;
				case floatarray://extend 浮点数组处理
					getsymdo;
					if(sym == lparen){
						getsymdo;
						if(sym == number){
							int offset=num;
							getsymdo;
							gendo(lod, lev-table[i].level, table[i].adr+offset,1);
							if(sym != rparen){
								error(51);
							}
						}
						else{
							error(50);
						} 
						 
					}
					else{
						error(49);	
					}
					break;
				}
			}
			getsymdo;
		}
		else if(sym==ceilsym){
			//printf("\nwhat ceil??\n");
			getsymdo;
			if(sym==lparen){
				getsymdo;
				if(sym==ident){
					i = position(id, *ptx); /* 查找名字 */
					if (i == 0)
					{
						error(11);  /* 标识符未声明 */
					}
					else{
						switch(table[i].kind){
							case constant:
								gendo(lit,0,(int)(table[i].val)+1,0);
								break;
							case variable:
								gendo(lod,lev-table[i].level,table[i].adr,2);
								break;
							case procedur:
								error(21);
								break;
						}
						getsymdo;
						if(sym == rparen){
							getsymdo;
						}else{
							error(22);
						}
					}
				}
			}else{
				error(48);//缺少左括号
			}
		}
		else if(sym == floorsym){
			getsymdo;
			if(sym==lparen){
				getsymdo;
				if(sym==ident){
					i = position(id, *ptx); /* 查找名字 */
					if (i == 0)
					{
						error(11);  /* 标识符未声明 */
					}
					else{
						switch(table[i].kind){
							case constant:
								gendo(lit,0,(int)table[i].val,0);
								break;
							case variable:
								//printf("\nwhat floor??\n");
								gendo(lod,lev-table[i].level,table[i].adr,0);
								break;
							case procedur:
								error(21);
								break;
						}
						getsymdo;
						if(sym == rparen){
							getsymdo;
						}else{
							error(22);
						}
					}
				}
			}else{
				error(48);//缺少左括号
			}
		}
		else if(sym == sumsym){//extend 数组求和函数
			getsymdo;
			if(sym==lparen){
				getsymdo;
				if(sym==ident){
					i = position(id, *ptx); /* 查找名字 */
					if (i == 0)
					{
						error(11);  /* 标识符未声明 */
					}else{
						if(table[i].kind==intarray){
							getsymdo;
							if(sym==lparen){// sum(a(3));形式
								getsymdo;
								if(sym==number){
									getsymdo;
									int offset = 0;
									//printf("数组长度：%d\n",num);
									for(offset;offset<=num-1;offset++){//数组里的值依次取到栈顶
										gendo(lod, lev-table[i].level, table[i].adr+offset,0);
									}
									for(offset=0;offset<=num-2;offset++){//数组值相加
										gendo(opr,0,2,0);
									}
									getsymdo;
									//gendo(lod,lev-table[i].level,table[i].adr,0);
									if(sym==rparen){
										getsymdo;
									}else{
										error(22);
									}
								}else{
									error(106);
								}
							}else{
								error(48);
							}
						}
						else if(table[i].kind==floatarray){
							getsymdo;
							if(sym==lparen){// sum(a(3));形式
								getsymdo;
								if(sym==number){
									getsymdo;
									int offset = 0;
									//printf("数组长度：%d\n",num);
									for(offset;offset<=num-1;offset++){//数组里的值依次取到栈顶
										gendo(lod, lev-table[i].level, table[i].adr+offset,1);
									}
									for(offset=0;offset<=num-2;offset++){//数组值相加
										gendo(opr,0,2,0);
									}
									getsymdo;
									//gendo(lod,lev-table[i].level,table[i].adr,0);
									if(sym==rparen){
										getsymdo;
									}else{
										error(22);
									}
								}else{
									error(106);
								}
							}else{
								error(48);
							}
						}
						else{
							error(108);
						}
					}
				}
			}else{
				error(48);//缺少左括号
			}
		}
		//avg
		else if(sym == avgsym){
			getsymdo;
			if(sym==lparen){
				getsymdo;
				if(sym==ident){
					i = position(id, *ptx); /* 查找名字 */
					if (i == 0)
					{
						error(11);  /* 标识符未声明 */
					}else{
						if(table[i].kind==intarray){
							getsymdo;
							if(sym==lparen){// avg(a(3));形式
								getsymdo;
								if(sym==number){
									getsymdo;
									int offset = 0;
									//printf("数组长度：%d\n",num);
									for(offset;offset<=num-1;offset++){//数组里的值依次取到栈顶
										gendo(lod, lev-table[i].level, table[i].adr+offset,0);
									}
									for(offset=0;offset<=num-2;offset++){//数组值相加
										gendo(opr,0,2,0);
									}
									gendo(lit,0,num,0);//存数组大小到栈顶
									gendo(opr,0,5,0);
									getsymdo;
									//gendo(lod,lev-table[i].level,table[i].adr,0);
									if(sym==rparen){
										getsymdo;
									}else{
										error(22);
									}
								}else{
									error(106);
								}
							}else{
								error(48);
							}
						}
						else if(table[i].kind==floatarray){
							getsymdo;
							if(sym==lparen){// avg(a(3));形式
								getsymdo;
								if(sym==number){
									getsymdo;
									int offset = 0;
									//printf("数组长度：%d\n",num);
									for(offset;offset<=num-1;offset++){//数组里的值依次取到栈顶
										gendo(lod, lev-table[i].level, table[i].adr+offset,1);
									}
									for(offset=0;offset<=num-2;offset++){//数组值相加
										gendo(opr,0,2,0);
									}
									gendo(lit,0,num,0);//存数组大小到栈顶
									gendo(opr,0,5,0);
									getsymdo;
									//gendo(lod,lev-table[i].level,table[i].adr,0);
									if(sym==rparen){
										getsymdo;
									}else{
										error(22);
									}
								}else{
									error(106);
								}
							}else{
								error(48);
							}
						}
						else{
							error(108);
						}
					}
				}
			}else{
				error(48);//缺少左括号
			}
		}
		else if(sym==maxsym){//extend 求数组最大值
			getsymdo;
			if(sym==lparen){
				getsymdo;
				if(sym==ident){
					i = position(id, *ptx); /* 查找名字 */
					if (i == 0)
					{
						error(11);  /* 标识符未声明 */
					}else{
						if(table[i].kind==intarray){
							getsymdo;
							if(sym==lparen){// max(a(3));形式
								getsymdo;
								if(sym==number){
									getsymdo;
									int offset = 0;
									//printf("数组长度：%d\n",num);
									for(offset;offset<=num-1;offset++){//数组里的值依次取到栈顶
										gendo(lod, lev-table[i].level, table[i].adr+offset,0);
									}
									for(offset=0;offset<=num-2;offset++){//数组值比较求最大值
										gendo(opr,0,18,0);
									}
									getsymdo;
									//gendo(lod,lev-table[i].level,table[i].adr,0);
									if(sym==rparen){
										getsymdo;
									}else{
										error(22);
									}
								}else{
									error(106);
								}
							}else{
								error(48);
							}
						}
						else if(table[i].kind==floatarray){
							getsymdo;
							if(sym==lparen){// max(a(3));形式
								getsymdo;
								if(sym==number){
									getsymdo;
									int offset = 0;
									//printf("数组长度：%d\n",num);
									for(offset;offset<=num-1;offset++){//数组里的值依次取到栈顶
										gendo(lod, lev-table[i].level, table[i].adr+offset,1);
									}
									for(offset=0;offset<=num-2;offset++){//数组值比较求最大值
										gendo(opr,0,18,0);
									}
									getsymdo;
									//gendo(lod,lev-table[i].level,table[i].adr,0);
									if(sym==rparen){
										getsymdo;
									}else{
										error(22);
									}
								}else{
									error(106);
								}
							}else{
								error(48);
							}
						}
						else{
							error(108);
						}
					}
				}
			}else{
				error(48);//缺少左括号
			}
		}
		else if(sym==minsym){//extend 求数组最小值
			getsymdo;
			if(sym==lparen){
				getsymdo;
				if(sym==ident){
					i = position(id, *ptx); /* 查找名字 */
					if (i == 0)
					{
						error(11);  /* 标识符未声明 */
					}else{
						if(table[i].kind==intarray){
							getsymdo;
							if(sym==lparen){// min(a(3));形式
								getsymdo;
								if(sym==number){
									getsymdo;
									int offset = 0;
									//printf("数组长度：%d\n",num);
									for(offset;offset<=num-1;offset++){//数组里的值依次取到栈顶
										gendo(lod, lev-table[i].level, table[i].adr+offset,0);
									}
									for(offset=0;offset<=num-2;offset++){//数组值比较求最小值
										gendo(opr,0,19,0);
									}
									getsymdo;
									//gendo(lod,lev-table[i].level,table[i].adr,0);
									if(sym==rparen){
										getsymdo;
									}else{
										error(22);
									}
								}else{
									error(106);
								}
							}else{
								error(48);
							}
						}
						else if(table[i].kind==floatarray){
							getsymdo;
							if(sym==lparen){// min(a(3));形式
								getsymdo;
								if(sym==number){
									getsymdo;
									int offset = 0;
									//printf("数组长度：%d\n",num);
									for(offset;offset<=num-1;offset++){//数组里的值依次取到栈顶
										gendo(lod, lev-table[i].level, table[i].adr+offset,1);
									}
									for(offset=0;offset<=num-2;offset++){//数组值比较求最小值
										gendo(opr,0,19,0);
									}
									getsymdo;
									//gendo(lod,lev-table[i].level,table[i].adr,0);
									if(sym==rparen){
										getsymdo;
									}else{
										error(22);
									}
								}else{
									error(106);
								}
							}else{
								error(48);
							}
						}
						else{
							error(108);
						}
					}
				}
			}else{
				error(48);//缺少左括号
			}
		}
		else
		{
			if(sym == number)   /* 因子为数 */
			{
				if (num > amax)
				{
					error(31);
					num = 0;
				}
				gendo(lit, 0, num,0);
				getsymdo;
			}
			else if(sym==floatsym){//extend 处理浮点因子
				//printf("破防了%f\n",floatNum);
				gendo(lit,1,0,floatNum);//f l a isFlow
				getsymdo;
			}
			else
			{
				if (sym == lparen)  /* 因子为表达式 */
				{
					getsymdo;
					memcpy(nxtlev, fsys, sizeof(bool)*symnum);
					nxtlev[rparen] = true;
					expressiondo(nxtlev, ptx, lev);
					if (sym == rparen)
					{
						getsymdo;
					}
					else
					{
						error(22);  /* 缺少右括号 */
					}
				}
				testdo(fsys, facbegsys, 23);    /* 因子后有非法符号 */
			}
		}
	}
	return 0;
}

/*
* 条件处理
*/
int condition(bool* fsys, int* ptx, int lev)
{
	enum symbol relop;
	bool nxtlev[symnum];

	if(sym == oddsym)   /* 准备按照odd运算处理 */
	{
		getsymdo;
		expressiondo(fsys, ptx, lev);
		gendo(opr, 0, 6,0);   /* 生成odd指令 */
	}
	else
	{
		/* 逻辑表达式处理 */
		memcpy(nxtlev, fsys, sizeof(bool)*symnum);
		nxtlev[eql] = true;
		nxtlev[neq] = true;
		nxtlev[lss] = true;
		nxtlev[leq] = true;
		nxtlev[gtr] = true;
		nxtlev[geq] = true;
		expressiondo(nxtlev, ptx, lev);
		if (sym!=eql && sym!=neq && sym!=lss && sym!=leq && sym!=gtr && sym!=geq)
		{
			error(20);
		}
		else
		{
			relop = sym;
			getsymdo;
			expressiondo(fsys, ptx, lev);
			switch (relop)
			{
			case eql:
				gendo(opr, 0, 8,0);
				break;
			case neq:
				gendo(opr, 0, 9,0);
				break;
			case lss:
				gendo(opr, 0, 10,0);
				break;
			case geq:
				gendo(opr, 0, 11,0);
				break;
			case gtr:
				gendo(opr, 0, 12,0);
				break;
			case leq:
				gendo(opr, 0, 13,0);
				break;
			}
		}
	}
	return 0;
}

/*
* 解释程序
*/
void interpret()
{
	int p, b, t;    /* 指令指针，指令基址，栈顶指针 */
	struct instruction i;   /* 存放当前指令 */
	float s[stacksize];   /* 栈 */
	float fs[stacksize]; //extend浮点数栈 一开始加的 后面没怎么用上但不太敢删掉

	printf("start pl0-G-Version\n");
	t = 0;
	b = 0;
	p = 0;
	s[0] = s[1] = s[2] = 0;
	fs[0] = fs[1] = fs[2] = 0;
	do {
		i = code[p];    /* 读当前指令 */
		p++;
		switch (i.f)
		{
		case lit:   /* 将a的值取到栈顶 */
			if(i.l==0){
				s[t] = i.a;
				t++;
			}else if(i.l==1){
				//printf("\n?why?%f\n",i.isFlow);
				s[t]=i.isFlow;
				t++;
			}
			// s[t] = i.a;
			//fs[t] = i.a;
			// t++;
			break;
		case opr:   /* 数学、逻辑运算 */
			switch (i.a)
			{
			case 0:
				t = b;
				p = s[t+2];
				b = s[t+1];
				break;
			case 1:
				s[t-1] = -s[t-1];
				break;
			case 2:
				t--;
				s[t-1] = s[t-1]+s[t];
				break;
			case 3:
				t--;
				s[t-1] = s[t-1]-s[t];
				break;
			case 4:
				t--;
				s[t-1] = s[t-1]*s[t];
				break;
			case 5:
				t--;
				s[t-1] = s[t-1]/s[t];
				break;
			case 6:
				s[t-1] = (int)(s[t-1])%2;
				break;
			case 8:
				t--;
				s[t-1] = (s[t-1] == s[t]);
				break;
			case 9:
				t--;
				s[t-1] = (s[t-1] != s[t]);
				break;
			case 10:
				t--;
				s[t-1] = (s[t-1] < s[t]);
				break;
			case 11:
				t--;
				s[t-1] = (s[t-1] >= s[t]);
				break;
			case 12:
				t--;
				s[t-1] = (s[t-1] > s[t]);
				break;
			case 13:
				t--;
				s[t-1] = (s[t-1] <= s[t]);
				break;
			case 14:
				if((int)(s[t-1]*10000)%10000==0){
					//printf("int:");
					printf("%d",(int)s[t-1]);
				}else{
					printf("%f", s[t-1]);
					//fprintf(fa2, "%d", s[t-1]);
				}
				t--;
				break;
			case 15:
				printf("\n");
				fprintf(fa2,"\n");
				break;
			case 16://change 输入浮点
				printf("?");
				fprintf(fa2, "?");
				scanf("%f", &(s[t]));
				fprintf(fa2, "%d\n", s[t]);
				t++;
				break;
			case 17://extend 输入整型
				printf("?");
				fprintf(fa2, "?");
				int temp;
				scanf("%d",&temp);
				s[t]=temp;
				// scanf("%d", &(s[t]));
				fprintf(fa2, "%d\n", s[t]);
				t++;
				break;
			case 18://extend 栈顶和次栈顶比较，将最大值存在次栈顶，t-1
				t--;
				if(s[t-1]<=s[t]){
					s[t-1]=s[t];
				}
				break;
			case 19://extend 栈顶和次栈顶比较，将最小值存在次栈顶，t-1
				t--;
				if(s[t-1]>=s[t]){
					s[t-1]=s[t];
				}
				break;
			}
			break;
		case lod:   /* 取相对当前过程的数据基地址为a的内存的值到栈顶 */
			// s[t] = s[base(i.l,s,b)+i.a];
			// t++;
			if((((int)i.isFlow)%10)==0){
				s[t] = (int)s[base(i.l,s,b)+i.a];
				t++;
			}else if ((((int)i.isFlow)%10)==1){
				s[t] = s[base(i.l,s,b)+i.a];
				t++;
			}else if((((int)i.isFlow)%10)==2){
				//extend 向上取整
				//printf("ceil!\n");
				s[t] = (int)s[base(i.l,s,b)+i.a]+1;
				t++;
			}
			break;
		case sto:   /* 栈顶的值存到相对当前过程的数据基地址为a的内存 */
			// t--;
			// s[base(i.l, s, b) + i.a] = s[t];
			if((((int)i.isFlow)%10)==0){
				t--;
				s[base(i.l, s, b) + i.a] = (int)s[t];
				// printf("值：%d\n",(int)s[t]);
			}else if ((((int)i.isFlow)%10)==1){
				t--;
				s[base(i.l, s, b) + i.a] = s[t];
			}
			break;
		case cal:   /* 调用子过程 */
			s[t] = base(i.l, s, b); /* 将父过程基地址入栈 */
			s[t+1] = b; /* 将本过程基地址入栈，此两项用于base函数 */
			s[t+2] = p; /* 将当前指令指针入栈 */
			b = t;  /* 改变基地址指针值为新过程的基地址 */
			p = i.a;    /* 跳转 */
			break;
		case inte:  /* 分配内存 */
			t += i.a;
			break;
		case jmp:   /* 直接跳转 */
			p = i.a;
			break;
		case jpc:   /* 条件跳转 */
			t--;
			if (s[t] == 0)
			{
				p = i.a;
			}
			break;
		}
	} while (p != 0);
}

/* 通过过程基址求上l层过程的基址 */
int base(int l, float* s, int b)
{
	int b1;
	b1 = b;
	while (l > 0)
	{
		b1 = s[b1];
		l--;
	}
	return b1;
}


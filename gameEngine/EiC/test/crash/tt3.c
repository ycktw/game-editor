#include <stdio.h>
#include <time.h>


#if 1
  
int main(int z, int p, int q, int r);

int a[1817];

int main(int z, int p, int q, int r)
{
    printf("yesss %d %d %d %d\n",z,p,q,r);
    for(p=80;q+p-80;p-=2*a[p])
	for(z=9;z--;)
	     q=3&(r=time(0) +r*57)/7,q=q ?
		 q-1 ? q-2 ? 1-p%79 ? -1 : 0 : p%79-77?1 : 0
		     : p<1659?79:0:p>158?-79: 0,q ?
			 !a[p+q*2]?a[p+=a[p+=q]=q]=q:0:0;
    for(;q++-1817;)
        printf(q%79?"%c":"%c\n"," #"[!a[q-1]]);
    
    return 0;
}


#else


int a[1817];

int t(int z, int p, int q, int r)
{
    char *str = " #";
    for(p=80;q+p-80;p-=2*a[p])
         for(z=9;z--;)
	     q=3&(r=time(0)+r*57)/7,q=q ?
		 q-1 ? q-2 ? 1-p%79 ? -1 : 0
				 : p%79-77 ?
				     1:0:p<1659?79:0:p>158?-79:0,q?!a[p+q*2] ?
					 a[p+=a[p+=q]=q]=q:0:0;
    for(;q++-1817;)
        printf(q%79?"%c":"%c\n",str[!a[q-1]]);
    
    return 0;
}

#define L(x)  putchar(x)
int t2(int z, int p, int q, int r)
{
    int i;
    char *str = " #";
    p = 80;

    for(i=0;i<sizeof(a)/sizeof(int);++i)
	a[i] = 0;

   /* for(p=80;q+p-80;p-=2*a[p])*/
	for(z=9;z--;) {
	    printf("%d %d %d %d\n",z,p,q,r);
	    q=3&(r=100000+r*57)/7,q=q ?
		q-1 ? q-2 ? 1-p%79 ? -1 : 0
		    : p%79-77 ?
			1:0:p<1659?79:0:p>158?-79:0,q?!a[p+q*2] ?
			    printf("[%d,%d]\n",p,q), a[p+=a[p+=q]=q]=q:0:0;
	
	    printf("%d %d %d %d -----\n",z,p,q,r);
	}
    printf("--- %d %d %d %d %d-----\n",z,p,q,r,a[p]);

    return 0;
}


void main()
{
    t2(1, -1073743592, -1073743584, -1073743467);
}


#endif









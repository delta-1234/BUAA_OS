#include <stdio.h>
int main() {
	int n;
	scanf("%d", &n);
	int temp = n;
	int x =0;
	while(temp!=0){
		x=x*10;
		x+=temp%10;
		temp/=10;
	}

	if (x==n) {
		printf("Y\n");
	} else {
		printf("N\n");
	}
	return 0;
}

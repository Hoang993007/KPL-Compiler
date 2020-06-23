#include<stdio.h>
#include<string.h>


int main(){
	float a=atof("2.");
	printf("%f", a);
	
	 float val;
   char str[20];
   
   strcpy(str, "98993489");
   val = atof(str);
   printf("String value = %s, Float value = %f\n", str, val);
}

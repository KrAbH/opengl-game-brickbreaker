#include<iostream>
#include<cstdio>

long long int arr[1000100];
using namespace std;
int main()
{
	long long int n, t, i, j ;
	cin >> t;
	while(t--)
	{
		j=0;
		cin >> n;
		for(i=0;i<n;i++)
			cin >> arr[i];
		printf("1 ");
		for(i=1;i<n;i++)
		{
			if(arr[i]>=arr[i-1])
			{
				j++;
			}
			else
				j=0;

			printf("%lld ", j);
		}
	}
	return 0;
}

#define NULL 0
main()
{
	char	*strchr();

	if (strchr("abc", 'c') == NULL)
		printf("error 1\n");
	if (strchr("abc", 'd') != NULL)
		printf("error 2\n");
	if (strchr("abc", 'a') == NULL)
		printf("error 3\n");
	if (strchr("abc", 'c') == NULL)
		printf("error 4\n");
}



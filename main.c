#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include <stdio.h>

int ft_strlen(char *str);

typedef struct s_cmd
{
	char			*path;
	char			**args;
	struct s_prog	*next;
} t_cmd;

typedef struct s_series
{
	t_cmd			*cmd;
	struct s_series	*next;
} t_series;


void exit_fatal(void)
{
	write("error: fatal", 2, 12);
	exit(1);
}

t_series	*create_series(void)
{
	t_series *series;

	series = malloc(sizeof(t_series));
	if (series == NULL)
		exit_fatal();
	series->cmd = NULL;
	series->next = NULL;
	return (series);
}

t_cmd	*create_cmd(void)
{
	t_cmd	*cmd = malloc(sizeof(t_cmd));

	if (cmd == NULL)
		exit_fatal();
	cmd->path = NULL;
	cmd->args = NULL;
	cmd->next = NULL;
	return (cmd);
}

t_series *parse_argv(char **argv)
{
	int i = 1;
	t_series 	*series;
	t_series 	*curr_series;
	t_cmd		*curr_cmd;

	if (argv[i] == NULL)
		return (NULL);

	series = create_series();
	curr_series = series;
	curr_cmd = NULL;
	while (argv[i])
	{
		
		if (strcmp(argv[i], ";") == 0) // Когда встретили новую серию команд после ;
		{
			curr_series->next = create_series();
			curr_series = curr_series->next;
		}
		else if (strcmp(argv[i], "|") == 0) // Когда встретили пайп
		{

		}
		else // Если встретили команду или аргумент
		{
			if (curr_cmd == NULL)
			{
				curr_cmd = create_cmd();
				curr_cmd->path = argv[i];
				
			}
		}
		i++;
	}
	
}

int ft_strlen(char *str)
{
	int n = 0;
	while (*str)
	{
		n++;
		str++;
	}
	return (n);
}
void print_text(char **text)
{
	while (*text)
	{
		write(1, *text, ft_strlen(*text));
		write(1, "\n", 1);
		text++;
	}
}



int my_cd(char *path)
{
	if (chdir(path) == -1)
	{
		write(2, "error: cd: cannot change directory to ", 38);
		write(2, path, ft_strlen(path));
		write(1, "\n", 1);
		return (1);
	}
	return (0);
}

int main(int argc, char **argv, char **envp)
{
	print_text(argv);
	write(1, "\nDONE\n", 6);
	return 0;
	char *str = malloc(128);
	while (1)
	{
		scanf("%s", str);
		printf("cd exit code = %d\n", my_cd(str));
	}
	
}
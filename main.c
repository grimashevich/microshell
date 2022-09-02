#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include <stdio.h>


typedef struct s_cmd
{
	char			**args;
	struct s_cmd	*next;
} t_cmd;

typedef struct s_series
{
	t_cmd			*cmd;
	struct s_series	*next;
} t_series;


int ft_strlen(char *str);
void exit_fatal(void);
void print_cmd(t_cmd *cmd);
void print_args(char **args);


int get_text_len(char **text)
{
	int i = 0;

	if (text == NULL)
		return (0);
	while (text[i])
		i++;
	return (i);
}

char **add_str_to_text(char **text, char *str)
{
	int		text_len;
	int		i = 0;
	char	**new_text;

	text_len = get_text_len(text);
	new_text = malloc(sizeof(char *) * (text_len + 2));
	if (! new_text)
		exit_fatal();
	while (i < text_len)
	{
		new_text[i] = text[i];
		i++;
	}
	new_text[i] = str;
	new_text[i + 1] = NULL;
	if (text)
		free(text);
	return (new_text);
}

void exit_fatal(void)
{
	write(2, "error: fatal\n", 13);
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
	cmd->args = NULL;
	cmd->next = NULL;
	return (cmd);
}


t_series	*parse_argv(char **argv)
{
	int i = 1;
	t_series *start_series = create_series();
	t_series *curr_seies = start_series;
	t_cmd *curr_cmd = create_cmd();
	if (! start_series || ! curr_cmd)
		exit_fatal();
	curr_seies->cmd = curr_cmd;
	while (argv[i])
	{
		if (argv[i][0] == ';')
		{
			curr_seies->next = create_series();
			curr_seies = curr_seies->next;
			curr_cmd = create_cmd();
			curr_seies->cmd = curr_cmd;
		}
		else if (argv[i][0] == '|')
		{
			curr_cmd->next = create_cmd();
			curr_cmd = curr_cmd->next;
		}
		else
		{
			curr_cmd->args = add_str_to_text(curr_cmd->args, argv[i]);
		}
		i++;		
	}
	return (start_series);
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

int get_cmd_count(t_cmd *cmd)
{
	int i = 0;
	while (cmd)
	{
		cmd = cmd->next;
		i++;
	}
	return (i);
}

void close_pipes(int fd[2][2])
{
	close(fd[0][0]);
	close(fd[0][1]);
	close(fd[1][0]);
	close(fd[1][1]);
}

void exec_cmd(t_cmd *cmd, char **envp)
{
	int 	*pids;
	int 	i = 0;
	int		pipes[2][2];

	pids = malloc(sizeof(int) * get_cmd_count(cmd));

	while (cmd)
	{
		print_args(cmd->args);
		pipe(pipes[i % 2]);
		pids[i] = fork();
		if (pids[i] == 0)
		{
			if (i == 0)
			{
				dup2(pipes[i % 2][1], 1);
				close(pipes[i % 2][0]);
				close(pipes[! (i % 2)][0]);
				close(pipes[! (i % 2)][1]);
			}
			else if (cmd->next != NULL)
			{
				dup2(pipes[i % 2][1], 1);
				dup2(pipes[!(i % 2)][0], 0);
				close(pipes[i % 2][0]);
				close(pipes[!(i % 2)][1]);
			}
			else
			{
				dup2(pipes[! (i % 2)][0], 0);
				close(pipes[i % 2][0]);
				close(pipes[i % 2][1]);
				close(pipes[! (i % 2)][1]);
			}
			execve(cmd->args[0], cmd->args, envp);
			exit_fatal();
		}
		else
		{
			close(pipes[! (i % 2)][0]);
			close(pipes[! (i % 2)][1]);	
		}
		i++;
		cmd = cmd->next;
	}
	close_pipes(pipes);
//TODO waitpid
}


void executor(t_series *series, char **envp)
{
	int	i = 0;
	while (series)
	{
		exec_cmd(series->cmd, envp);
		series = series->next;
	}
	
}

void print_args(char **args)
{
	int i = 0;
	while (args[i])
	{
		printf("%s ", args[i++]);
	}
	printf("\n");
}

void print_cmd(t_cmd *cmd)
{
	
	while (cmd)
	{
		print_args(cmd->args);
		if (cmd->next != NULL)
			printf("- - - PIPE - - -\n");
		else
			printf("\n");
		cmd = cmd->next;
	}
}

void print_series(t_series *series)
{
	int i = 0;
	while (series)
	{
		printf("\nSERIES: %d\n", i);
		print_cmd(series->cmd);
		series = series->next;
		i++;
	}
	
}


int main(int argc, char **argv, char **envp)
{
	t_series	*series = parse_argv(argv);
	executor(series, envp);
	return (0);
	write(1, "\nDONE\n", 6);
	return 0;
	char *str = malloc(128);
	while (1)
	{
		scanf("%s", str);
		printf("cd exit code = %d\n", my_cd(str));
	}

	return (0);	
}



/* 

./a.out /bin/echo 111 "|" /bin/echo 222 "|" /bin/echo 333
 */
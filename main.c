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

void exit_execve(char *exec_name)
{
	write(2, "error: cannot execute ", 23);
	write(2, exec_name, ft_strlen(exec_name));
	write(2, "\n", 1);
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

void close_pipes(int fd[2][2], int i)
{
	close(fd[0][0]);
	close(fd[0][1]);
	if (i > 0)
	{
		close(fd[1][0]);
		close(fd[1][1]);
	}
}

void error_cd(void)
{
	write(2, "error: cd: bad arguments\n", 26);
}

void exec_cmd(t_cmd *cmd, char **envp)
{
	int 	*pids;
	int 	i = 0;
	int		pipes[2][2];

	if (strcmp(cmd->args[0], "cd") == 0)
	{
		if (get_text_len(cmd->args) != 2)
		{
			error_cd();
			return;
		}
		my_cd(cmd->args[1]);
		return;
	}

	pids = malloc(sizeof(int) * get_cmd_count(cmd));

	while (cmd)
	{
		pipe(pipes[i % 2]);
		pids[i] = fork();
		if (pids[i] == 0)
		{
			if (i == 0)
			{
				if (cmd->next != NULL)
					dup2(pipes[i % 2][1], 1);
				else
					close(pipes[i % 2][1]);
				close(pipes[i % 2][0]);
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
				close(pipes[!(i % 2)][1]);
			}
			if (execve(cmd->args[0], cmd->args, envp) == -1)
			{
				exit_execve(cmd->args[0]);
			}
			exit_fatal();
		}
		else // main process
		{
			if (i > 0)
			{
				close(pipes[! (i % 2)][0]);
				close(pipes[! (i % 2)][1]);	
			}
		}
		i++;
		cmd = cmd->next;

	}
	close_pipes(pipes, i);
	while (i >= 0)
		waitpid(pids[i--], NULL, 0);
	free(pids);
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

void free_cmd(t_cmd *cmd)
{
	t_cmd *tmp;
	while (cmd)
	{
		tmp = cmd;
		cmd = cmd->next;
		free(tmp->args);
		free(tmp);
	}
	
}

void free_series(t_series *series)
{
	t_series *tmp;
	
	while (series)
	{
		tmp = series;
		series = series->next;
		free_cmd(tmp->cmd);
		free(tmp);
	}
}

int main(int argc, char **argv, char **envp)
{
	t_series	*series = parse_argv(argv);
	executor(series, envp);
	free_series(series);
	return (0);	
}



/* 

leaks --atExit -- ./a.out /bin/ls "|" /usr/bin/grep t "|" /usr/bin/wc ";" /bin/echo strnig 2 ";" /bin/ls ";" /bin/echo 1 2 3 4 5
 
 ./microshell /bin/ls a.out "|" /usr/bin/grep micro "|" /bin/cat -n ; /bin/echo dernier ; /bin/echo
 */
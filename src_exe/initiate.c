/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   initiate.c                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: pamatya <pamatya@student.42heilbronn.de    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/12/17 21:46:09 by pamatya           #+#    #+#             */
/*   Updated: 2025/04/08 12:31:01 by pamatya          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../include/minishell.h"

void		init_shell(t_shell *shl, int ac, char **av, char **envp);
void		start_shell(t_shell *shl);
char		*get_input(t_shell *shell, const char *prompt);
void		clearout(t_shell *shl);

static void	update_history(t_shell *shell);

/*
Initializes the elements of the shell struct "t_shell"
  - Copies the environment variables to the shell struct
  - Copies the PATH variable to the shell struct
  - Updates the SHLVL variable
  - Sets the current working directory
  - Sets the prompt
  - Frees all malloc's and exits the program if any of the above steps fail
*/
void	init_shell(t_shell *shl, int ac, char **av, char **envp)
{
	*shl = (t_shell){
		.ac = ac,
		.av = av,
		.stdio[0] = dup(STDIN_FILENO),
		.stdio[1] = dup(STDOUT_FILENO),
		.shlvl = 1,
		.tmp_file_fd = -1
	};
	if (shl->stdio[0] < 0 || shl->stdio[1] < 0)
	{
		if (shl->stdio[1] != -1)
			close(shl->stdio[1]);
		if (shl->stdio[0] != -1)
			close(shl->stdio[0]);
		exit_early(NULL, NULL, ERRMSG_DUP);
	}
	init_environ_variables(shl, envp);
	update_shlvl(shl);
	shl->cwd = getcwd(NULL, 0);
	if (!shl->cwd)
		exit_early(shl, NULL, "getcwd");
	set_prompt(shl, "", " % ");
}

/*
Function to start shell execution
*/
void	start_shell(t_shell *shl)
{
	while (1)
	{
		shl->exit_code_prev = shl->exit_code;
		reset_cmd_vars(shl, 1, 0);
		set_signal(shl);
		tty_echo_sig(shl, false);
		shl->cmdline = get_input(shl, shl->prompt);
		if (!shl->cmdline)
			break ;
		if (parser(shl))
		{
			update_history(shl);
			continue ;
		}
		update_history(shl);
		retokenize_args(shl, shl->cmds_lst);
		index_cmds(shl);
		set_env_paths(shl);
		if (shl->cmds_lst)
			mini_execute(shl);
	}
}

char	*get_input(t_shell *shell, const char *prompt)
{
	char	*input;
	char	*line;

	if (isatty(STDIN_FILENO))
		input = readline(prompt);
	else
	{
		line = get_next_line(STDIN_FILENO);
		if (!line)
			return (NULL);
		input = ft_strtrim(line, "\n");
		free(line);
		if (!input)
			exit_early(shell, NULL, ERRMSG_MALLOC);
	}
	return (input);
}

static void	update_history(t_shell *shell)
{
	if (*shell->cmdline
		&& compare_strings(shell->cmdline, shell->prev_cmdline, 1) < 1)
		add_history(shell->cmdline);
	if (shell->prev_cmdline)
		free(shell->prev_cmdline);
	shell->prev_cmdline = ft_strdup(shell->cmdline);
	if (!shell->prev_cmdline)
		exit_early(shell, NULL, ERRMSG_MALLOC);
}

/*
Function to free any allocated memory during the minishell session and return
  - When called, the fn frees all allocated t_shell field pointers, clears the 
	readline history, and return control to the calling function.
  - This function is meant to exit minishell without memory leaks from pointers
	accesible from the main stack.
  - The funciton will not free any memory allocated locally by the calling fn
	which is not accessible through the main stack.
  - The function also does not free any t_cmds struct nodes with the assumption
	that this will occur within the minishell session at the end of each command
	cycle.
*/
void	clearout(t_shell *shl)
{
	if (shl->environ != NULL)
		ft_free2d_safe(&shl->environ);
	if (shl->variables != NULL)
		ft_lst_free(&shl->variables);
	if (shl->local_vars != NULL)
		ft_lst_free(&shl->local_vars);
	if (shl->home_dir != NULL)
		ft_free_safe((void **)(&(shl->home_dir)));
	if (shl->owd != NULL)
		ft_free_safe((void **)(&(shl->owd)));
	if (shl->cwd != NULL)
		ft_free_safe((void **)(&(shl->cwd)));
	if (shl->prompt != NULL)
		ft_free_safe((void **)(&(shl->prompt)));
	rl_clear_history();
	if (shl->stdio[0] != STDIN_FILENO && shl->stdio[0] != -1)
	{
		if (close(shl->stdio[0]) < 0)
			exit_early(NULL, NULL, ERRMSG_CLOSE);
	}
	if (shl->stdio[1] != STDOUT_FILENO && shl->stdio[1] != -1)
	{
		if (close(shl->stdio[1]) < 0)
			exit_early(NULL, NULL, ERRMSG_CLOSE);
	}
}

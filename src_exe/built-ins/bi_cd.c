/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   bi_cd.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: pamatya <pamatya@student.42heilbronn.de    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/09/26 14:42:30 by pamatya           #+#    #+#             */
/*   Updated: 2025/04/08 12:30:56 by pamatya          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/minishell.h"

void		mini_cd(t_shell *shl, t_cmds *cmd);

static int	get_new_cwd(t_shell *shl, char *arg, char **new_cwd);
static int	get_new_cwd_helper(t_shell *shl, char *arg, char **new_cwd);
static void	update_old_wdir(t_shell *shl);
static void	update_wdir(t_shell *shl);

/*
Built-in cd function
*/
void	mini_cd(t_shell *shl, t_cmds *cmd)
{
	char	*new_cwd;
	char	*arg;

	update_env_var(shl, cmd, UNDERSCORE, NULL);
	arg = *(cmd->args + cmd->skip + 1);
	if (get_new_cwd(shl, arg, &new_cwd) == 0)
	{
		if (chdir(new_cwd) < 0)
		{
			ft_fprintf_str(STDERR_FILENO, (const char *[]){ERSHL, ERRMSG_CD,
				new_cwd, ": ", strerror(errno), "\n", NULL});
			cmd->exit_code = ERRCODE_GENERAL;
			return ;
		}
		new_cwd = getcwd(NULL, 0);
		update_old_wdir(shl);
		update_wdir(shl);
		free(new_cwd);
		if (compare_strings(arg, "-", 1) == 1)
			printf("%s\n", shl->cwd);
		free(shl->prompt);
		set_prompt(shl, "", " % ");
	}
	else
		cmd->exit_code = ERRCODE_GENERAL;
}

// Helper static fn for mini_cd() fn
static int	get_new_cwd(t_shell *shl, char *arg, char **new_cwd)
{
	t_lst_str	*node;

	if (arg == NULL || compare_strings(arg, "--", 1))
	{
		node = ft_find_node(shl->variables, "HOME", 0, 1);
		if (!node)
			return (shl->exit_code = 1, ft_fprintf_str(STDERR_FILENO,
					(const char *[]){ERSHL, "cd: HOME not set\n", NULL}), -1);
		*new_cwd = node->val;
	}
	else
		if (get_new_cwd_helper(shl, arg, new_cwd) == -1)
			return (-1);
	return (shl->exit_code = 0, 0);
}

// Helper static function for get_new_cwd() fn
static int	get_new_cwd_helper(t_shell *shl, char *arg, char **new_cwd)
{
	t_lst_str	*node;

	if (compare_strings(arg, "-", 1))
	{
		node = ft_find_node(shl->variables, "OLDPWD", 0, 1);
		if (node && node->val)
			*new_cwd = node->val;
		else if ((!node || !(node->val)) && shl->owd)
			*new_cwd = shl->owd;
		else if ((!node || !(node->val)) && !shl->owd)
			return (shl->exit_code = 1, ft_fprintf_str(STDERR_FILENO,
					(const char *[]){ERSHL, "cd: OLDPWD not set\n", NULL}), -1);
	}
	else if (compare_strings(arg, ".", 1))
		*new_cwd = shl->cwd;
	else
		*new_cwd = arg;
	return (0);
}

/*
Function to update env variables pwd and oldpwd
*/
static void	update_old_wdir(t_shell *shl)
{
	char		*old_pwd;
	t_lst_str	*cur_pwd_node;
	t_lst_str	*old_pwd_node;

	old_pwd_node = ft_find_node(shl->variables, "OLDPWD", 0, 1);
	cur_pwd_node = ft_find_node(shl->variables, "PWD", 0, 1);
	if (!cur_pwd_node)
		old_pwd = ft_strjoin("OLDPWD=", shl->cwd);
	else
		old_pwd = ft_strjoin("OLDPWD=", cur_pwd_node->val);
	if (!old_pwd)
		exit_early(shl, NULL, ERRMSG_MALLOC);
	if (old_pwd_node)
	{
		add_to_environ(shl, old_pwd, 0);
		store_as_variable(shl, old_pwd, 0);
	}
	free(old_pwd);
}

/*
Function to update env variables pwd and oldpwd
*/
static void	update_wdir(t_shell *shl)
{
	char		*new_pwd;
	t_lst_str	*cur_pwd_node;

	cur_pwd_node = ft_find_node(shl->variables, "PWD", 0, 1);
	if (shl->owd)
		free(shl->owd);
	shl->owd = shl->cwd;
	shl->cwd = getcwd(NULL, 0);
	if (!shl->cwd)
		exit_early(shl, NULL, ERRMSG_MALLOC);
	new_pwd = ft_strjoin("PWD=", shl->cwd);
	if (!new_pwd)
		exit_early(shl, NULL, ERRMSG_MALLOC);
	if (cur_pwd_node)
	{
		add_to_environ(shl, new_pwd, 0);
		store_as_variable(shl, new_pwd, 0);
	}
	free(new_pwd);
}

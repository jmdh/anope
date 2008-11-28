/* OperServ core functions
 *
 * (C) 2003-2008 Anope Team
 * Contact us at info@anope.org
 *
 * Please read COPYING and README for further details.
 *
 * Based on the original code of Epona by Lara.
 * Based on the original code of Services by Andy Church.
 *
 * $Id$
 *
 */
/*************************************************************************/

#include "module.h"

int do_operumodes(User * u);
void myOperServHelp(User * u);

class OSUMode : public Module
{
 public:
	OSUMode(const std::string &modname, const std::string &creator) : Module(modname, creator)
	{
		Command *c;

		this->SetAuthor("Anope");
		this->SetVersion("$Id$");
		this->SetType(CORE);

		c = createCommand("UMODE", do_operumodes, is_services_root, OPER_HELP_UMODE, -1, -1, -1, -1);
		this->AddCommand(OPERSERV, c, MOD_UNIQUE);

		this->SetOperHelp(myOperServHelp);

		if (!ircd->umode)
			throw ModuleException("Your IRCd does not support setting umodes");
	}
};

/**
 * Unload the module
 **/
void AnopeFini()
{

}


/**
 * Add the help response to anopes /os help output.
 * @param u The user who is requesting help
 **/
void myOperServHelp(User * u)
{
	if (is_services_admin(u) && u->isSuperAdmin) {
		notice_lang(s_OperServ, u, OPER_HELP_CMD_UMODE);
	}
}

/**
 * Change any user's UMODES
 *
 * modified to be part of the SuperAdmin directive -jester
 * check user flag for SuperAdmin -rob
 * @param u The user who issued the command
 * @param MOD_CONT to continue processing other modules, MOD_STOP to stop processing.
 */
int do_operumodes(User * u)
{
	char *nick = strtok(NULL, " ");
	char *modes = strtok(NULL, "");

	User *u2;

	/* Only allow this if SuperAdmin is enabled */
	if (!u->isSuperAdmin) {
		notice_lang(s_OperServ, u, OPER_SUPER_ADMIN_ONLY);
		return MOD_CONT;
	}

	if (!nick || !modes) {
		syntax_error(s_OperServ, u, "UMODE", OPER_UMODE_SYNTAX);
		return MOD_CONT;
	}

	/**
	 * Only accept a +/- mode string
	 *-rob
	 **/
	if ((modes[0] != '+') && (modes[0] != '-')) {
		syntax_error(s_OperServ, u, "UMODE", OPER_UMODE_SYNTAX);
		return MOD_CONT;
	}
	if (!(u2 = finduser(nick))) {
		notice_lang(s_OperServ, u, NICK_X_NOT_IN_USE, nick);
	} else {
		ircdproto->SendMode(findbot(s_OperServ), nick, "%s", modes);

		common_svsmode(u2, modes, NULL);

		notice_lang(s_OperServ, u, OPER_UMODE_SUCCESS, nick);
		notice_lang(s_OperServ, u2, OPER_UMODE_CHANGED, u->nick);

		if (WallOSMode)
			ircdproto->SendGlobops(s_OperServ, "\2%s\2 used UMODE on %s",
							 u->nick, nick);
	}
	return MOD_CONT;
}

MODULE_INIT("os_umode", OSUMode)

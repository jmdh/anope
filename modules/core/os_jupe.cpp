/* OperServ core functions
 *
 * (C) 2003-2010 Anope Team
 * Contact us at team@anope.org
 *
 * Please read COPYING and README for further details.
 *
 * Based on the original code of Epona by Lara.
 * Based on the original code of Services by Andy Church.
 */

/*************************************************************************/

#include "module.h"

class CommandOSJupe : public Command
{
 public:
	CommandOSJupe() : Command("JUPE", 1, 2, "operserv/jupe")
	{
	}

	CommandReturn Execute(User *u, const std::vector<Anope::string> &params)
	{
		Anope::string jserver = params[0];
		Anope::string reason = params.size() > 1 ? params[1] : "";
		Server *server = Server::Find(jserver);

		if (!isValidHost(jserver, 3))
			notice_lang(Config.s_OperServ, u, OPER_JUPE_HOST_ERROR);
		else if (server && (server == Me || server == Me->GetLinks().front()))
			notice_lang(Config.s_OperServ, u, OPER_JUPE_INVALID_SERVER);
		else
		{
			Anope::string rbuf = "Juped by " + u->nick + (!reason.empty() ? ": " + reason : "");
			if (server)
				ircdproto->SendSquit(jserver, rbuf);
			Server *juped_server = new Server(Me, jserver, 1, rbuf, ircd->ts6 ? ts6_sid_retrieve() : "");
			juped_server->SetFlag(SERVER_JUPED);
			ircdproto->SendServer(juped_server);

			if (Config.WallOSJupe)
				ircdproto->SendGlobops(OperServ, "\2%s\2 used JUPE on \2%s\2", u->nick.c_str(), jserver.c_str());
		}
		return MOD_CONT;
	}

	bool OnHelp(User *u, const Anope::string &subcommand)
	{
		notice_help(Config.s_OperServ, u, OPER_HELP_JUPE);
		return true;
	}

	void OnSyntaxError(User *u, const Anope::string &subcommand)
	{
		syntax_error(Config.s_OperServ, u, "JUPE", OPER_JUPE_SYNTAX);
	}

	void OnServHelp(User *u)
	{
		notice_lang(Config.s_OperServ, u, OPER_HELP_CMD_JUPE);
	}
};

class OSJupe : public Module
{
	CommandOSJupe commandosjupe;

 public:
	OSJupe(const Anope::string &modname, const Anope::string &creator) : Module(modname, creator)
	{
		this->SetAuthor("Anope");
		this->SetType(CORE);

		this->AddCommand(OperServ, &commandosjupe);
	}
};

MODULE_INIT(OSJupe)
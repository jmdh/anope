/* BotServ core fantasy functions
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

int do_fantasy(int argc, char **argv);

class BSFantasySeen : public Module
{
 public:
	BSFantasySeen(const std::string &modname, const std::string &creator) : Module(modname, creator)
	{
		EvtHook *hook;

		this->SetAuthor("Anope");
		this->SetVersion("$Id$");
		this->SetType(CORE);

		hook = createEventHook(EVENT_BOT_FANTASY, do_fantasy);
		this->AddEventHook(hook);
	}
};

/**
 * Unload the module
 **/
void AnopeFini()
{

}

/**
 * Handle seen fantasy command.
 * @param argc Argument count
 * @param argv Argument list
 * @return MOD_CONT or MOD_STOP
 **/
int do_fantasy(int argc, char **argv)
{
	User *u;
	ChannelInfo *ci;
	User *u2;
	NickAlias *na;
	ChanAccess *access;
	char buf[BUFSIZE];
	char *target = NULL;

	if (argc < 4)
		return MOD_CONT;

	if (stricmp(argv[0], "seen") == 0) {
		u = finduser(argv[1]);
		ci = cs_findchan(argv[2]);
		if (!u || !ci)
			return MOD_CONT;

		target = myStrGetToken(argv[3], ' ', 0);

		if (stricmp(ci->bi->nick, target) == 0) {
			/* If we look for the bot */
			snprintf(buf, sizeof(buf), getstring(u->na, BOT_SEEN_BOT),
					 u->nick);
			ircdproto->SendPrivmsg(ci->bi, ci->name, "%s", buf);
		} else if (!(na = findnick(target)) || (na->status & NS_VERBOTEN)) {
			/* If the nick is not registered or forbidden */
			snprintf(buf, sizeof(buf), getstring(u->na, BOT_SEEN_UNKNOWN),
					 target);
			ircdproto->SendPrivmsg(ci->bi, ci->name, "%s", buf);
		} else if ((u2 = nc_on_chan(ci->c, na->nc))) {
			/* If the nick we're looking for is on the channel,
			 * there are three possibilities: it's yourself,
			 * it's the nick we look for, it's an alias of the
			 * nick we look for.
			 */
			if (u == u2 || (u->na && u->na->nc == na->nc))
				snprintf(buf, sizeof(buf), getstring(u->na, BOT_SEEN_YOU),
						 u->nick);
			else if (!stricmp(u2->nick, target))
				snprintf(buf, sizeof(buf),
						 getstring(u->na, BOT_SEEN_ON_CHANNEL), u2->nick);
			else
				snprintf(buf, sizeof(buf),
						 getstring(u->na, BOT_SEEN_ON_CHANNEL_AS), target,
						 u2->nick);

			ircdproto->SendPrivmsg(ci->bi, ci->name, "%s", buf);
		} else if ((access = get_access_entry(na->nc, ci))) {
			/* User is on the access list but not present actually.
			   Special case: if access->last_seen is 0 it's that we
			   never seen the user.
			 */
			if (access->last_seen) {
				char durastr[192];
				duration(u->na, durastr, sizeof(durastr),
						 time(NULL) - access->last_seen);
				snprintf(buf, sizeof(buf), getstring(u->na, BOT_SEEN_ON),
						 target, durastr);
			} else {
				snprintf(buf, sizeof(buf),
						 getstring(u->na, BOT_SEEN_NEVER), target);
			}
			ircdproto->SendPrivmsg(ci->bi, ci->name, "%s", buf);
		} else if (na->nc == ci->founder) {
			/* User is the founder of the channel */
			char durastr[192];
			duration(u->na, durastr, sizeof(durastr),
					 time(NULL) - na->last_seen);
			snprintf(buf, sizeof(buf), getstring(u->na, BOT_SEEN_ON),
					 target, durastr);
			ircdproto->SendPrivmsg(ci->bi, ci->name, "%s", buf);
		} else {
			/* All other cases */
			snprintf(buf, sizeof(buf), getstring(u->na, BOT_SEEN_UNKNOWN),
					 target);
			ircdproto->SendPrivmsg(ci->bi, ci->name, "%s", buf);
		}
		/* free myStrGetToken(ed) variable target (#851) */
		if (target) delete [] target;
	}

	return MOD_CONT;
}

MODULE_INIT("bs_fantasy_seen", BSFantasySeen)

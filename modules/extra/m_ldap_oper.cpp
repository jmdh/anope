#include "module.h"
#include "modules/ldap.h"

static std::set<Oper *> my_opers;
static Anope::string opertype_attribute;

class IdentifyInterface : public LDAPInterface
{
	std::map<LDAPQuery, Anope::string> requests;

 public:
	IdentifyInterface(Module *m) : LDAPInterface(m)
	{
	}

	void Add(LDAPQuery id, const Anope::string &nick)
	{
		this->requests[id] = nick;
	}

	void OnResult(const LDAPResult &r) override
	{
		std::map<LDAPQuery, Anope::string>::iterator it = this->requests.find(r.id);
		if (it == this->requests.end())
			return;
		User *u = User::Find(it->second);
		this->requests.erase(it);


		if (!u || !u->Account())
			return;

		NickServ::Account *nc = u->Account();

		try
		{
			const LDAPAttributes &attr = r.get(0);

			const Anope::string &opertype = attr.get(opertype_attribute);

			OperType *ot = OperType::Find(opertype);
			if (ot != NULL && (nc->o == NULL || ot != nc->o->ot))
			{
				Oper *o = nc->o;
				if (o != NULL && my_opers.count(o) > 0)
				{
					my_opers.erase(o);
					delete o;
				}
				o = new Oper(u->nick, ot);
				my_opers.insert(o);
				nc->o = o;
<<<<<<< HEAD
				Log(this->owner) << "m_ldap_oper: Tied " << u->nick << " (" << nc->GetDisplay() << ") to opertype " << ot->GetName();
=======
				Log(this->owner) << "Tied " << u->nick << " (" << nc->display << ") to opertype " << ot->GetName();
>>>>>>> 2.0
			}
		}
		catch (const LDAPException &ex)
		{
			if (nc->o != NULL)
			{
				if (my_opers.count(nc->o) > 0)
				{
					my_opers.erase(nc->o);
					delete nc->o;
				}
				nc->o = NULL;

<<<<<<< HEAD
				Log() << "Removed services operator from " << u->nick << " (" << nc->GetDisplay() << ")";
=======
				Log(this->owner) << "Removed services operator from " << u->nick << " (" << nc->display << ")";
>>>>>>> 2.0
			}
		}
	}

	void OnError(const LDAPResult &r) override
	{
		this->requests.erase(r.id);
	}
};

class LDAPOper : public Module
	, public EventHook<Event::NickIdentify>
	, public EventHook<Event::DelCore>
{
	ServiceReference<LDAPProvider> ldap;
	IdentifyInterface iinterface;

	Anope::string binddn;
	Anope::string password;
	Anope::string basedn;
	Anope::string filter;
 public:
	LDAPOper(const Anope::string &modname, const Anope::string &creator) : Module(modname, creator, EXTRA | VENDOR)
		, ldap("LDAPProvider", "ldap/main")
		, iinterface(this)
	{

	}

	void OnReload(Configuration::Conf *conf) override
	{
		Configuration::Block *config = Config->GetModule(this);

		this->binddn = config->Get<Anope::string>("binddn");
		this->password = config->Get<Anope::string>("password");
		this->basedn = config->Get<Anope::string>("basedn");
		this->filter = config->Get<Anope::string>("filter");
		opertype_attribute = config->Get<Anope::string>("opertype_attribute");

		for (std::set<Oper *>::iterator it = my_opers.begin(), it_end = my_opers.end(); it != it_end; ++it)
			delete *it;
		my_opers.clear();
	}

	void OnNickIdentify(User *u) override
	{
		try
		{
			if (!this->ldap)
				throw LDAPException("No LDAP interface. Is m_ldap loaded and configured correctly?");
			else if (this->basedn.empty() || this->filter.empty() || opertype_attribute.empty())
				throw LDAPException("Could not search LDAP for opertype settings, invalid configuration.");

			if (!this->binddn.empty())
				this->ldap->Bind(NULL, this->binddn.replace_all_cs("%a", u->Account()->GetDisplay()), this->password.c_str());
			LDAPQuery id = this->ldap->Search(&this->iinterface, this->basedn, this->filter.replace_all_cs("%a", u->Account()->GetDisplay()));
			this->iinterface.Add(id, u->nick);
		}
		catch (const LDAPException &ex)
		{
			Log() << ex.GetReason();
		}
	}

	void OnDelCore(NickServ::Account *nc) override
	{
		if (nc->o != NULL && my_opers.count(nc->o) > 0)
		{
			my_opers.erase(nc->o);
			delete nc->o;
			nc->o = NULL;
		}
	}
};

MODULE_INIT(LDAPOper)

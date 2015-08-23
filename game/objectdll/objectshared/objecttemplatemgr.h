//////////////////////////////////////////////////////////////////////////////
// Object Template handling code

#ifndef __OBJECTTEMPLATEMGR_H__
#define __OBJECTTEMPLATEMGR_H__

#include <hash_map>


class ObjectTemplateMgrHashCompare
{
public:

	// parameters for hash table
	enum
	{
		bucket_size = 4,	// 0 < bucket_size
		min_buckets = 8		// min_buckets = 2 ^^ N, 0 < N
	};

	ObjectTemplateMgrHashCompare()
	{
	}

	size_t 		operator()(const std::string &pKey) const
	{
		// hash _Keyval to size_t value
		return Convert(pKey);
	}

	bool 		operator()(const std::string &pKey1, const std::string &pKey2) const
	{
		// test if _Keyval1 ordered before _Keyval2
		return Compare(pKey1, pKey2);
	}

private:

	size_t		Convert(const std::string &pKey) const
	{
		uint32 nHash = 0;

		const char *pName = &(*pKey.begin());
		for (; *pName; ++pName)
		{
			nHash = 13 * nHash + (toupper(*pName) - '@');
		}

		return nHash;
	}

	bool 		Compare(const std::string &lhs, const std::string &rhs) const
	{
		return stricmp(lhs.c_str(), rhs.c_str()) < 0;
	}
};



class CObjectTemplateMgr
{
public:
	CObjectTemplateMgr();
	~CObjectTemplateMgr();

	void AddTemplate(const ObjectCreateStruct *pOCS);
	const ObjectCreateStruct *FindTemplate(const char *pName) const;
	void Clear();

	void Save( ILTMessage_Write *pMsg );
	void Load( ILTMessage_Read *pMsg );

protected:

#ifdef __MINGW32__
    typedef __gnu_cxx::hash_map< std::string, ObjectCreateStruct, ObjectTemplateMgrHashCompare > TTemplateMap;
#else
	typedef stdext::hash_map< std::string, ObjectCreateStruct, ObjectTemplateMgrHashCompare > TTemplateMap;
#endif

	TTemplateMap m_cTemplates;
};

#endif //__OBJECTTEMPLATEMGR_H__

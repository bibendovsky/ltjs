#include "stdafx.h"
#include "templatelist.h"

int s_cLTLinkBankRefCount = 0;
CBankedList<LTLink>* s_pLTLinkBank = LTNULL;

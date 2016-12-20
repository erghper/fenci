#include "wordbase.h"

CWordBase::CWordBase()
{
	m_emType    = WORD_TYPE_ASCII;
	m_pWordRoot = NULL;
}

CWordBase::~CWordBase()
{
	Close();
}

bool CWordBase::Init(const char* pDictFile, int nPoolSize)
{
	string strLine;
	char   szLine[50] = {'\0'};
	
	_RuneLinkNode* pCurrTempNode = NULL;
	
	//��ʼ��Rune�ڴ��
	m_objRunePool.Init(nPoolSize);	
	m_objNodePool.Init(nPoolSize);
	
	//��ʼ����
	m_pWordRoot = m_objNodePool.Create();
	
	ifstream ifs(pDictFile);
	while(!ifs.eof())
	{
		getline(ifs, strLine);
		pCurrTempNode = m_pWordRoot;
		
		//printf("[CWordBase::Init]Line Begin(%s).\n", strLine.c_str());
		int nLen = 0;
		sprintf(szLine, "%s", strLine.c_str());
		if(szLine[strLine.length() - 1] == 0x0D)
		{
#ifdef WIN32
			nLen = strlen(szLine) + 1;
#else
			nLen = strlen(szLine);
#endif 
		}
		else
		{
#ifdef WIN32
			nLen = strlen(szLine) + 2;
#else
			nLen = strlen(szLine) + 1;
#endif 			
		}
		
		for(int i = 0; i < nLen - 1;)
		{
			_Rune* pRune = m_objRunePool.Create();
			if(NULL == pRune)
			{
				printf("[CWordBase::Init]RunePool is finished.\n");
				return false;
			}
			
			ENUM_WORD_TYPE emType = Get_Rune_From_String(szLine, i, nLen, pRune);
			if(emType != WORD_TYPE_UNKNOW)
			{
				i = i + pRune->m_nRuneLen;
				if(m_emType == WORD_TYPE_ASCII && emType != WORD_TYPE_ASCII)
				{
					//��¼��ǰ�ַ���
					m_emType = emType;
				}
			}
			else
			{
				printf("<ERROR WORD TYPE>");
				break;
			}	
			
			pCurrTempNode = Set_HashMap_Word_Tree(pCurrTempNode, pRune);

			if(i == nLen - 1)
			{
				memcpy(pCurrTempNode->m_pWord, szLine, nLen - 1);
				//printf("[CWordBase::Init]m_pWord=%s.\n", pCurrTempNode->m_pWord);
			}					

		}
	}
	
	//չʾ��������
	//int nLayer = 0;
	//DisplayTempNodeList(m_pWordRoot, nLayer);
	
	return true;
}

void CWordBase::DisplayTempNodeList(_RuneLinkNode* pRuneNode, int nLayer)
{
	//�ݹ���ʾ�����ӽڵ�
	int nMapSize = (int)pRuneNode->m_hmapRuneNextMap.size();
	//printf("[CWordBase::DisplayTempNodeList]pRuneNode=%d,size=%d.\n", nLayer, nMapSize);
	if(nMapSize > 0)
	{
		nLayer++;
		for(_RuneLinkNode::hmapRuneNextMap::iterator b = pRuneNode->m_hmapRuneNextMap.begin(); b != pRuneNode->m_hmapRuneNextMap.end(); b++)
		{
			pRuneNode = (_RuneLinkNode* )b->second;
			
			if(pRuneNode->m_hmapRuneNextMap.size() > 0)
			{
				if(NULL != pRuneNode->m_pRune)
				{
					printf("<%d>", nLayer);
					pRuneNode->m_pRune->DisPlay();
					printf("-->");
				}					
			}
			
			if(strlen(pRuneNode->m_pWord) > 0 && pRuneNode->m_hmapRuneNextMap.size() > 0)
			{
				printf("1.Value=%s.\n", pRuneNode->m_pWord);
			}				
						
			DisplayTempNodeList(pRuneNode, nLayer);				
		}
	}
	else
	{
		if(NULL != pRuneNode->m_pRune)
		{
			printf("<%d>", nLayer);
			pRuneNode->m_pRune->DisPlay();
			printf("-->");
		}
		if(strlen(pRuneNode->m_pWord) > 0)
		{
			printf("2.Value=%s.\n", pRuneNode->m_pWord);
		}
	}
}

void CWordBase::Close()
{
	//ɾ���ʻ���	
}

_RuneLinkNode* CWordBase::Set_HashMap_Word_Tree(_RuneLinkNode* pRuneNode, _Rune* pRune)
{
	_RuneLinkNode::hmapRuneNextMap::iterator f = pRuneNode->m_hmapRuneNextMap.find(pRune->Get_Number());
	if(f != pRuneNode->m_hmapRuneNextMap.end())
	{
		_RuneLinkNode* pCurrRuneNode = (_RuneLinkNode* )f->second;
		if((*pCurrRuneNode->m_pRune) == (*pRune))
		{
			//����ҵ��ˣ��򷵻ص�ǰ�ڵ�
			m_objRunePool.Delete(pRune);
			return pCurrRuneNode;
		}
	}
	
	//���û�ҵ����򴴽��µ�
	//_RuneLinkNode* pNode = new _RuneLinkNode();
	_RuneLinkNode* pNode = m_objNodePool.Create();
	pNode->m_pRune = pRune;
	pRuneNode->m_hmapRuneNextMap.insert(pair<int, _RuneLinkNode*>(pRune->Get_Number(), pNode));
	return pNode;
}

ENUM_WORD_TYPE CWordBase::Get_Rune_From_String(const char* pWord, int nBegin, int nLen, _Rune* pRune)
{
	unsigned char cBegin = (unsigned char)pWord[nBegin];
	//printf("******0x%02x******\n", cBegin);
	if(cBegin < 0x80)
	{
		//��ǰ��ascii�ַ���
		pRune->m_szRune[0] = (unsigned char)pWord[nBegin];
		pRune->m_nRuneLen  = 1;
		pRune->m_emType    = WORD_TYPE_ASCII;
		pRune->Set_Number();
		return pRune->m_emType;
	}
	else
	{
		//�Ƚ��ַ���4λ
		char szTemp = cBegin >> 4;
		if(szTemp == 0xe && nLen - nBegin >= 3)
		{
			//Ϊ���ֽ�utf8��ʽ
			pRune->m_szRune[0] = (unsigned char)pWord[nBegin];
			pRune->m_szRune[1] = (unsigned char)pWord[nBegin + 1];
			pRune->m_szRune[2] = (unsigned char)pWord[nBegin + 2];
			pRune->m_nRuneLen = 3;
			pRune->m_emType   = WORD_TYPE_UTF8;
			pRune->Set_Number();
			return pRune->m_emType;
		}
		//else if(((char)(szTemp & 0xc) == 0xc) && (nLen - nBegin >= 2))
		else if(nLen - nBegin >= 2)
		{
			//Ϊgbk����
			pRune->m_szRune[0] = (unsigned char)pWord[nBegin];
			pRune->m_szRune[1] = (unsigned char)pWord[nBegin + 1];
			pRune->m_nRuneLen = 2;
			pRune->m_emType   = WORD_TYPE_GBK;
			pRune->Set_Number();
			return pRune->m_emType;
		}
		else
		{
			return pRune->m_emType;
		}
	}
}

_RuneLinkNode* CWordBase::Find(_Rune* pRune, _RuneLinkNode* pRuneLinkNode)
{
	if(pRuneLinkNode->m_hmapRuneNextMap.size() == 0)
	{
		//�ҵ���ĩβ
		if((*pRune) == (*pRuneLinkNode->m_pRune))
		{
			return pRuneLinkNode;
		}
		else
		{
			return NULL;
		}
	}
	
	//printf("[CWordBase::Find]pRune=%d, size=%d.\n", pRune->Get_Number(), pRuneLinkNode->m_hmapRuneNextMap.size());
	_RuneLinkNode::hmapRuneNextMap::iterator f = pRuneLinkNode->m_hmapRuneNextMap.find(pRune->Get_Number());
	if(f == pRuneLinkNode->m_hmapRuneNextMap.end())
	{
		//û���ҵ���ǰ�ַ�
		return NULL;
	}
	else
	{
		return f->second;	
	}
}

int CWordBase::Cut_Word(const char* pSentence, vector<string>& vecWord)
{
	int i    = 0;
	int nPos = 0;
	int nSentenceLen = strlen(pSentence);
	
	_RuneLinkNode* pPosRuneLinkNode = m_pWordRoot;
	
	for(int i = 0; i < nSentenceLen;)
	{
		_Rune objRune;
		
		//printf("[CWordBase::Cut]i=%d.\n", i);
		ENUM_WORD_TYPE emType = Get_Rune_From_String(pSentence, i, nSentenceLen, &objRune);
		if(emType != WORD_TYPE_UNKNOW)
		{
			i = i + objRune.m_nRuneLen;
			if(emType != WORD_TYPE_ASCII && emType != m_emType)
			{
				printf("[CWordBase::Cut_Word]Dictory and Sentence character set mismatching.\n");
				return -1;
			}
		}
		else
		{
			printf("[CWordBase::Cut]<ERROR WORD TYPE>");
			break;
		}
		
		//printf("[CWordBase::Cut]");
		//objRune.DisPlay();
		//printf("\n");
		
		//printf("[CWordBase::Cut]pPosRuneLinkNode=0x%08x.\n", pPosRuneLinkNode);
		
		_RuneLinkNode* pCurrRuneLinkNode = Find(&objRune, pPosRuneLinkNode);
		if(NULL == pCurrRuneLinkNode)
		{
			//printf("[CWordBase::Cut]pCurrRuneLinkNode=NULL.\n");
			if(NULL != pPosRuneLinkNode && strlen(pPosRuneLinkNode->m_pWord) > 0)
			{
				string strWord = (string)pPosRuneLinkNode->m_pWord;
				vecWord.push_back(strWord);
			} 
			
			pPosRuneLinkNode = m_pWordRoot;
			//���´Ӵʸ���ʼ����
			pCurrRuneLinkNode = Find(&objRune, pPosRuneLinkNode);
			if(NULL != pCurrRuneLinkNode)
			{
				pPosRuneLinkNode = pCurrRuneLinkNode;
			}
		}
		else
		{
			if(i == nSentenceLen)
			{
				//�Ѿ��ҵ�ĩβ�ˣ�ֱ�����
				pPosRuneLinkNode = pCurrRuneLinkNode;
				string strWord = (string)pPosRuneLinkNode->m_pWord;
				vecWord.push_back(strWord);				
			}	
			else
			{
				pPosRuneLinkNode = pCurrRuneLinkNode;
			}
		}
	}
	
	return 0;
}

int CWordBase::Cut(const char* pSentence, vector<string>& vecWord)
{
	int i    = 0;
	int nPos = 0;
	int nSentenceLen = strlen(pSentence);
	
	char szTemp[MAX_WORD_LENGTH] = {'\0'};
	
	_RuneLinkNode* pPosRuneLinkNode = m_pWordRoot;
	
	for(int i = 0; i < nSentenceLen;)
	{
		_Rune objRune;
		
		//printf("[CWordBase::Cut]i=%d.\n", i);
		ENUM_WORD_TYPE emType = Get_Rune_From_String(pSentence, i, nSentenceLen, &objRune);
		if(emType != WORD_TYPE_UNKNOW)
		{
			i = i + objRune.m_nRuneLen;
			if(emType != WORD_TYPE_ASCII && emType != m_emType)
			{
				printf("[CWordBase::Cut]Dictory and Sentence character set mismatching.\n");
				return -1;
			}			
		}
		else
		{
			printf("[CWordBase::Cut]<ERROR WORD TYPE>");
			break;
		}
		
		//printf("[CWordBase::Cut]");
		//objRune.DisPlay();
		//printf("\n");
		
		//printf("[CWordBase::Cut]pPosRuneLinkNode=0x%08x.\n", pPosRuneLinkNode);
		
		_RuneLinkNode* pCurrRuneLinkNode = Find(&objRune, pPosRuneLinkNode);
		if(NULL == pCurrRuneLinkNode)
		{
			//printf("[CWordBase::Cut]pCurrRuneLinkNode=NULL.\n");
			if(NULL != pPosRuneLinkNode && strlen(pPosRuneLinkNode->m_pWord) > 0)
			{
				string strWord = (string)pPosRuneLinkNode->m_pWord;
				vecWord.push_back(strWord);
			} 
			
			pPosRuneLinkNode = m_pWordRoot;
			//���´Ӵʸ���ʼ����
			pCurrRuneLinkNode = Find(&objRune, pPosRuneLinkNode);
			if(NULL != pCurrRuneLinkNode)
			{
				pPosRuneLinkNode = pCurrRuneLinkNode;
			}
			else
			{
				memcpy(szTemp, objRune.m_szRune, objRune.m_nRuneLen);
				szTemp[objRune.m_nRuneLen] = '\0';
				vecWord.push_back((string)szTemp);
			}
		}
		else
		{
			//printf("[CWordBase::Cut]i=%d,nSentenceLen=%d.\n", i, nSentenceLen);
			if(i == nSentenceLen)
			{
				//�Ѿ��ҵ�ĩβ�ˣ�ֱ�����
				pPosRuneLinkNode = pCurrRuneLinkNode;
				string strWord = (string)pPosRuneLinkNode->m_pWord;
				vecWord.push_back(strWord);				
			}	
			else
			{
				pPosRuneLinkNode = pCurrRuneLinkNode;
			}
		}
	}	
	
	return 0;
}

int CWordBase::Add_Word(const char* pWord)
{
	char   szLine[50] = {'\0'};

	_RuneLinkNode* pCurrTempNode = NULL;	

	pCurrTempNode = m_pWordRoot;
	
	int nLen = 0;
	sprintf(szLine, "%s", pWord);
	if(szLine[strlen(pWord) - 1] == 0x0D)
	{
#ifdef WIN32
		nLen = strlen(szLine) + 1;
#else
		nLen = strlen(szLine);
#endif 
	}
	else
	{
#ifdef WIN32
		nLen = strlen(szLine) + 2;
#else
		nLen = strlen(szLine) + 1;
#endif 			
	}
	
	for(int i = 0; i < nLen - 1;)
	{
		_Rune* pRune = m_objRunePool.Create();
		if(NULL == pRune)
		{
			printf("[CWordBase::Init]RunePool is finished.\n");
			return false;
		}
		
		ENUM_WORD_TYPE emType = Get_Rune_From_String(szLine, i, nLen, pRune);
		if(emType != WORD_TYPE_UNKNOW)
		{
			i = i + pRune->m_nRuneLen;
			if(emType != WORD_TYPE_ASCII && emType != m_emType)
			{
				printf("[CWordBase::Cut_Word]Dictory and Sentence character set mismatching.\n");
				return -1;
			}
		}
		else
		{
			printf("<ERROR WORD TYPE>");
			break;
		}	
		
		pCurrTempNode = Set_HashMap_Word_Tree(pCurrTempNode, pRune);

		if(i == nLen - 1)
		{
			memcpy(pCurrTempNode->m_pWord, szLine, nLen - 1);
			//printf("[CWordBase::Init]m_pWord=%s.\n", pCurrTempNode->m_pWord);
		}
	}	
	
	//��ʾ���еĴʿ�
	//int nLayer = 0;
	//DisplayTempNodeList(m_pWordRoot, nLayer);	
	
	return 0;
}

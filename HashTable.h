#ifndef HASH_TABLE_H
#define HASH_TABLE_H

#include <stdio.h>
#include <ctype.h>
#include <string.h>

enum EM_HASH_STATE
{
	EM_SELECT = 0,
	EM_INSERT,
};

//hash��ṹ
template <class T>
struct _Hash_Table_Cell 
{
	char  m_cExists;                       //��ǰ���Ƿ��Ѿ�ʹ��,1�Ѿ�ʹ�ã�0û�б�ʹ��
	char* m_szKey;                         //��ǰ��keyֵ��û����Ϊ��
	int   m_nKeySize;                      //��ǰkey���ݳ���
	int   m_nNextKeyIndex;                 //������Ϣ����������г�ͻ,��¼��һ����ͻ������λ��
	int   m_nProvKeyIndex;                 //������Ϣ����������г�ͻ,��¼��һ����ͻ������λ��
	unsigned long m_uHashA;                //�ڶ��ε�hashkeyֵ
	unsigned long m_uHashB;                //�����ε�hashkeyֵ 
	T*    m_pValue;                        //��ǰ������ָ��
	int   m_nValueSize;                    //��ǰ�����峤��
	
	_Hash_Table_Cell()
	{
		Init();
	}
	
	void Init()
	{
		m_cExists       = 0;
		m_nKeySize      = 0;
		m_nValueSize    = 0;
		m_uHashA        = 0;
		m_uHashB        = 0;
		m_nNextKeyIndex = -1;
		m_nProvKeyIndex = -1;
		m_szKey         = NULL;
		m_pValue        = NULL;		
	}
	
	void Set_Key(char* pKey, int nKeySize)
	{
		m_szKey         = pKey;
		m_nKeySize      = nKeySize;
	}
	
	void Set_Value(T* pValue, int nValueSize)
	{
		m_pValue        = pValue;
		m_nValueSize    = nValueSize;
	}
	
	void Clear()
	{
		m_cExists       = 0;
		m_uHashA        = 0;
		m_uHashB        = 0;
		//m_nNextKeyIndex = -1;
		//m_nProvKeyIndex = -1;		
		if(NULL != m_szKey)	
		{
			memset(m_szKey, 0, m_nKeySize);		
		}
		if(NULL != m_pValue)
		{
			m_pValue = NULL;
		}
	}  
};

//hashTable��
template <class T>
class CHashTable
{
public:
	CHashTable()
	{
		m_lpTable    = NULL;
		m_nCount     = 0;
		m_nUsed      = 0;
		m_nKeySize   = 0;
		m_nValueSize = 0;
		
		memset(m_cryptTable, 0, sizeof(m_cryptTable));
		
		//��ʼ���������ݴ�
		prepareCryptTable();
	}	
	
	~CHashTable()
	{
		Close();
	}

	void Close()
	{
		if(NULL != m_lpTable)
		{
			delete[] m_lpTable;
			m_lpTable = NULL;
		}
		m_nCount = 0;
	}
	
	//����һ����֪���ڴ������(�����ʼ������)
	void Set_Base_Addr(char* pData, int nCount)
	{
		m_lpTable = (_Hash_Table_Cell<T>*)pData;
		m_nCount  = nCount;
		for(int i = 0; i < m_nCount; i++)
		{
			m_lpTable[i].Init();
		}
	}	
	
	//����һ��Hash Key�������(�����ʼ������)
	void Set_Base_Key_Addr(char* pData, int nSize, int nKeySize)
	{
		m_nKeySize    = nKeySize;
		int nCurrSize = 0;
		for(int i = 0; i < m_nCount; i++)
		{
			nCurrSize = nKeySize * (i + 1);
			if(nCurrSize <= nSize)
			{
				m_lpTable[i].Set_Key(pData + (nKeySize * i), nKeySize);
			}
			else
			{
				return;
			}
		}	
	}
	
	//����һ��Hash Value�ĵ�ַ��(�����ʼ������)
	void Set_Base_Value_Addr(char* pData, int nSize, int nValueSize)
	{
		m_nValueSize = nValueSize;
		int nCurrSize = 0;
		for(int i = 0; i < m_nCount; i++)
		{
			nCurrSize = nValueSize * (i + 1);
			if(nCurrSize <= nSize)
			{
				m_lpTable[i].Set_Value((T* )(pData + (nValueSize * i)), nValueSize);
			}
			else
			{
				return;
			}
		}				
	}
	
	//�õ���ǰ������ܸ���
	int Get_Count()
	{
		return (int)m_nCount;
	}	

	//�õ�����������ʹ�õĸ���
	int Get_Used_Count()
	{
		return m_nUsed;
	}
	
	//�õ����������ڴ��С
	size_t Get_Size()
	{
		return (sizeof(_Hash_Table_Cell<T>) + m_nKeySize + m_nValueSize) * m_nCount;
	}

	//�õ�����ָ��λ�õ�����
	T* Get_Index(int nIndex)
	{
		if(nIndex < 0 || nIndex > m_nCount - 1)
		{
			return NULL;
		}
		else
		{
			if(m_lpTable[nIndex].m_cExists == 1)
			{
				//����ʹ�ã���������
				return m_lpTable[nIndex].m_pValue;
			}
			else
			{
				//�����Ѿ���Ч
				return NULL;
			}
		}
	}

	int Set_Index_Clear(int nIndex)
	{
		if(nIndex < 0 || nIndex > m_nCount - 1)
		{
			return -1;
		}

		if(m_lpTable[nIndex].m_cExists == 0)
		{
			return -1;
		}
		else
		{
			m_lpTable[nIndex].Clear();
			if(m_nUsed >= 1)
			{
				m_nUsed--;
			}
			return nIndex;
		}
	}

	//����ָ��hash����λ�õ���ֵ
	int Set_Index(int nIndex, char* lpszString, T* pT)
	{
		const int HASH_A = 1, HASH_B = 2;
		if(nIndex < 0 || nIndex > m_nCount - 1)
		{
			return -1;
		}

		if(m_lpTable[nIndex].m_cExists == 1)
		{
			return -1;
		}
		else
		{
			m_lpTable[nIndex].m_cExists = 1;
			if(NULL != m_lpTable[nIndex].m_szKey)
			{
#ifndef WIN32
				sprintf(m_lpTable[nIndex].m_szKey, "%s", lpszString);
#else
				sprintf_s(m_lpTable[nIndex].m_szKey, m_lpTable[nIndex].m_nKeySize, "%s", lpszString);
#endif
			}
			m_lpTable[nIndex].m_uHashA = HashString(lpszString, HASH_A);
			m_lpTable[nIndex].m_uHashB = HashString(lpszString, HASH_B);
			m_lpTable[nIndex].m_pValue = pT;
			m_nUsed++;
			return nIndex;
		}
	}
	
	//���һ��Hash���ݿ�
	int Add_Hash_Data(const char* pKey, T* pValue)
	{
		if(NULL == m_lpTable)
		{
			//û���ҵ������ڴ�
			return -1;
		}
		
		int nPos = GetHashTablePos(pKey, EM_INSERT);
		if(-1 == nPos)
		{
			//�ڴ�����
			return -1;
		}
		else
		{
			m_lpTable[nPos].m_pValue = pValue;
			m_nUsed++;
			return nPos;
		}		
	}
	
	//���һ������ӳ���Ӧ��ֵ
	T* Get_Hash_Box_Data(const char* pKey)
	{
		if(NULL == m_lpTable)
		{
			//û���ҵ������ڴ�
			return NULL;
		}	
		
		int nPos = GetHashTablePos(pKey, EM_SELECT);
		if(-1 == nPos)
		{
			//û���ҵ�
			return NULL;
		}
		else
		{
			return m_lpTable[nPos].m_pValue;
		}			
	}
	
	//����һ��hash���ݿ�
	int Del_Hash_Data(const char* pKey)
	{
		return DelHashTablePos(pKey);
	}
	
private:
	//hash�㷨��Ӧ�ļ����ַ����ʵ�
	void prepareCryptTable()
	{
	  unsigned long seed = 0x00100001, index1 = 0, index2 = 0, i;
	  for(index1 = 0; index1 < 0x100; index1++)
	  { 
	    for(index2 = index1, i = 0; i < 5; i++, index2 += 0x100)
	    { 
	      unsigned long temp1, temp2;
	      seed = (seed * 125 + 3) % 0x2AAAAB;
	      temp1 = (seed & 0xFFFF) << 0x10;
	      seed = (seed * 125 + 3) % 0x2AAAAB;
	      temp2 = (seed & 0xFFFF);
	      if(index2 > 1280)
	      {
	      	printf("[prepareCryptTable]index2=%u.\n", (unsigned int)index2);
	      }
	      m_cryptTable[index2] = (char)(temp1 | temp2); 
	    } 
	  } 		
	}
	
	//����key��Ӧ��hash��ֵ
	unsigned long HashString(const char* pKey, unsigned long dwHashType)
	{
		unsigned char *key = (unsigned char *)pKey;
		unsigned long seed1 = 0x7FED7FED, seed2 = 0xEEEEEEEE;
		int ch;
		while(*key != 0)
		{
			ch = toupper(*key++);
			seed1 = m_cryptTable[(dwHashType << 8) + ch] ^ (seed1 + seed2);
			seed2 = ch + seed1 + seed2 + (seed2 << 5) + 3;
		}
		
		//printf("[HashString]seed1=%lu.\n", seed1);
		return seed1;		
	}
	
	//����֪�ĳ�ͻ������Ѱ�����һ��
	int GetLastClashKey(int nStartIndex, const char *lpszString, unsigned long uHashA, unsigned long uHashB, EM_HASH_STATE emHashState)
	{
		int nMaxIndex = m_nCount;
		int nRunCount = 0;
		
		//����֪������Ѱ�����һ��
		while(true)
		{
			if(nRunCount > nMaxIndex - 1)
			{
				return -1;
			}
			
			//printf("[GetLastClashKey](%s) curr(%d) next(%d)-->.\n", lpszString, nStartIndex, m_lpTable[nStartIndex].m_nNextKeyIndex);
			if(m_lpTable[nStartIndex].m_nNextKeyIndex == -1)
			{
				//�жϵ�ǰ�Ƿ��ǵ�ǰ����
				if(uHashA == m_lpTable[nStartIndex].m_uHashA && uHashB == m_lpTable[nStartIndex].m_uHashB)
				{
					//�ҵ���ƥ��Ķ��󣬷���֮
					/*
					printf("[GetLastClashKey][%d](%s).\n", 
												nStartIndex, m_lpTable[nStartIndex].m_szKey);
					*/
					printf("[GetLastClashKey]Search Count=%d.\n", nRunCount);			
					return nStartIndex;
				}			
				
				//����ǲ��룬��ȥ�ҿ��࣬������ǣ���ֱ�ӷ���
				if(emHashState == EM_INSERT)
				{
					//�ҵ�������ĩβ
					//��ʼѰ�ҿ����λ��
					//����ҿ���
					for(int i = nStartIndex + 1; i < m_nCount; i++)
					{
						if(m_lpTable[i].m_cExists == 0)
						{
							m_lpTable[i].m_cExists = 1;
							m_lpTable[i].m_uHashA  = uHashA;
							m_lpTable[i].m_uHashB  = uHashB;
							if(NULL != m_lpTable[i].m_szKey)
							{
#ifndef WIN32
								sprintf(m_lpTable[i].m_szKey, "%s", lpszString);
#else
								sprintf_s(m_lpTable[i].m_szKey, m_lpTable[i].m_nKeySize, "%s", lpszString);
#endif
							}
							//��¼������Ϣ
							m_lpTable[nStartIndex].m_nNextKeyIndex = i;
							m_lpTable[i].m_nProvKeyIndex           = nStartIndex;
							//printf("[GetLastClashKey](%s) <--prov(%d) next(%d)-->.\n", lpszString, nStartIndex, i);
							return i;
						}
					}
					
					//��ǰ�ҿ���
					for(int i = 0; i <= nStartIndex - 1; i++)
					{
						//��¼������Ϣ
						if(m_lpTable[i].m_cExists == 0)
						{
							m_lpTable[i].m_cExists = 1;
							m_lpTable[i].m_uHashA  = uHashA;
							m_lpTable[i].m_uHashB  = uHashB;
							if(NULL != m_lpTable[i].m_szKey)
							{
#ifndef WIN32
								sprintf(m_lpTable[i].m_szKey, "%s", lpszString);
#else
								sprintf_s(m_lpTable[i].m_szKey, m_lpTable[i].m_nKeySize, "%s", lpszString);
#endif
							}	

							m_lpTable[nStartIndex].m_nNextKeyIndex = i;
							m_lpTable[i].m_nProvKeyIndex           = nStartIndex;
							//printf("[GetLastClashKey](%s) <--prov(%d) next(%d)-->.\n", lpszString, nStartIndex, i);
							return i;
						}							
					}
				}
				
				return -1;
			}
			else
			{
				//�鿴��ǰ�ڵ��Ƿ��Ѿ����ͷ�
				if(emHashState == EM_INSERT)
				{
					if(m_lpTable[nStartIndex].m_cExists == 0)
					{
						m_lpTable[nStartIndex].m_cExists = 1;
						m_lpTable[nStartIndex].m_uHashA  = uHashA;
						m_lpTable[nStartIndex].m_uHashB  = uHashB;
						if(NULL != m_lpTable[nStartIndex].m_szKey)
						{
#ifndef WIN32
							sprintf(m_lpTable[nStartIndex].m_szKey, "%s", lpszString);
#else
							sprintf_s(m_lpTable[nStartIndex].m_szKey, m_lpTable[nStartIndex].m_nKeySize, "%s", lpszString);
#endif
						}	

						return nStartIndex;
					}
				}

				//����Ѱ��
				if(uHashA == m_lpTable[nStartIndex].m_uHashA && uHashB == m_lpTable[nStartIndex].m_uHashB)
				{
					//�ҵ���ƥ��Ķ��󣬷���֮
					printf("[GetLastClashKey]Search Count=%d.\n", nRunCount);				
					return nStartIndex;
				}
				
				/*
				printf("[GetLastClashKey][%d](%s) <--prov(%d) next(%d)-->.\n", 
								nStartIndex, m_lpTable[nStartIndex].m_szKey, 
								m_lpTable[nStartIndex].m_nProvKeyIndex, m_lpTable[nStartIndex].m_nNextKeyIndex);
				*/
				nStartIndex = m_lpTable[nStartIndex].m_nNextKeyIndex;	
			}
			
			nRunCount++;
		}		
	}
	
	//�õ�hashָ����λ��
	int GetHashTablePos(const char *lpszString, EM_HASH_STATE emHashState)
	{
		const int HASH_OFFSET = 0, HASH_A = 1, HASH_B = 2;
		unsigned long uHash = HashString(lpszString, HASH_OFFSET);
		unsigned long uHashA = HashString(lpszString, HASH_A);
		unsigned long uHashB = HashString(lpszString, HASH_B);
		unsigned long uHashStart = uHash % m_nCount, uHashPos = uHashStart;

		//printf("[GetHashTablePos]uHashPos=%d,m_nCount=%d,m_cExists=%d.\n", (int)uHashPos, m_nCount, (int)m_lpTable[uHashPos].m_cExists);
		
		if (m_lpTable[uHashPos].m_cExists == 0)
		{
			if(EM_INSERT == emHashState)
			{
				m_lpTable[uHashPos].m_cExists = 1;
				m_lpTable[uHashPos].m_uHashA  = uHashA;
				m_lpTable[uHashPos].m_uHashB  = uHashB;
				if(NULL != m_lpTable[uHashPos].m_szKey)
				{
#ifndef WIN32
					sprintf(m_lpTable[uHashPos].m_szKey, "%s", lpszString);
#else
					sprintf_s(m_lpTable[uHashPos].m_szKey, m_lpTable[uHashPos].m_nKeySize, "%s", lpszString);
#endif
				}

				//printf("[GetHashTablePos] return uHashPos=%d 1.\n", (int)uHashPos);
				return (int)uHashPos;
			}
			else
			{
				return  GetLastClashKey((int)uHashStart, lpszString, uHashA, uHashB, emHashState);
			}
					
		}
		else if(m_lpTable[uHashPos].m_uHashA == uHashA && m_lpTable[uHashPos].m_uHashB == uHashB)	
		{
			//�������hashֵ��ȣ���ֱ�ӷ���
			//printf("[GetHashTablePos] return uHashPos=%d 2.\n", (int)uHashPos);
			return (int)uHashPos;
		}
		else
		{
			int nPos = GetLastClashKey((int)uHashStart, lpszString, uHashA, uHashB, emHashState);
			//printf("[GetHashTablePos]key=%s (%d) -> (%d)  .\n", lpszString, uHashStart, nPos);
			return nPos;
		}
		
		//printf("[GetHashTablePos]no find.\n");
		return -1; //Error value		
	}
	
	int DelHashTablePos(const char *lpszString)
	{
		int nPos = GetHashTablePos(lpszString, EM_SELECT);
		if(-1 == nPos)
		{
			 GetHashTablePos(lpszString, EM_SELECT);
			return -1;
		}
		else
		{
			if(-1 != m_lpTable[nPos].m_nProvKeyIndex)
			{
				m_lpTable[m_lpTable[nPos].m_nProvKeyIndex].m_nNextKeyIndex = m_lpTable[nPos].m_nNextKeyIndex;
			}
			
			if(-1 != m_lpTable[nPos].m_nNextKeyIndex)
			{
				m_lpTable[m_lpTable[nPos].m_nNextKeyIndex].m_nProvKeyIndex = m_lpTable[nPos].m_nProvKeyIndex;
			}			
			
			m_lpTable[nPos].Clear();
			if(m_nUsed >= 1)
			{
				m_nUsed--;
			}
			return nPos;
		}			
	}
	
private:
	_Hash_Table_Cell<T>* m_lpTable;	
	char                 m_cryptTable[1280];
	int                  m_nUsed;
	int                  m_nCount;
	int                  m_nKeySize;
	int                  m_nValueSize;
};


#endif

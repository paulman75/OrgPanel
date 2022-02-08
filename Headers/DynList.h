template<class T> class DynArray
{
private:
	int m_Count;
	void *beg;

public:
	int count() const
	{	
		return m_Count;
	};
	
	void Add(T* newt, int Index)
	{
		if (Index>m_Count) return;

		int un=sizeof(T);
		void *b=malloc(un*(m_Count+1));
		void *p=b;
		if (m_Count==0)
		{
			memcpy(p,newt,un);
			m_Count++;
			beg=b;
			return;
		}
		if (Index>0)
		{
			memcpy(b,beg,un*Index);
			p=(void*)((int)p+un*Index);
		}
		memcpy(p,newt,un);
		p=(void*)((int)p+un);
		if (Index<m_Count) memcpy(p,(void*)((int)beg+Index*un),un*(m_Count-Index));
		free(beg);
		beg=b;
		m_Count++;
	};
	void Add(T* newt)
	{
		Add(newt, m_Count);
	}
	void Del(int i)
	{
		if (i>=m_Count) return;

		if (m_Count==1)
		{
			free(beg);
			beg=NULL;
			m_Count--;
			return;
		}
		int un=sizeof(T);
		void *b=malloc(un*(m_Count-1));
		void *p=b;

		if (i>0)
		{
			memcpy(p,beg,un*i);
			p=(void*)((int)p+un*i);
		}
		if (i<m_Count-1) memcpy(p,(void*)((int)beg+un*(i+1)),un*(m_Count-i-1));
		free(beg);
		beg=b;
		m_Count--;
	};
	void Clear()
	{
		if (!m_Count) return;
		free(beg);
		m_Count=0;
	}
	void SaveToFile(HANDLE hFile)
	{
		DWORD dw;
		WriteFile(hFile,&m_Count,4,&dw,NULL);
		if (m_Count)
		{
			byte *t=(byte*)malloc(sizeof(T)*m_Count);
			memcpy(t,beg,sizeof(T)*m_Count);
			byte sum=100;
			for (int i=0; i<(int)(m_Count*sizeof(T)); i++)
			{
				byte newb=t[i]+sum;
				sum+=t[i];
				t[i]=newb;
			}
			WriteFile(hFile,t,sizeof(T)*m_Count,&dw,NULL);
			free(t);
		}

	}
	void LoadFromFile(HANDLE hFile)
	{
		Clear();
		DWORD dw;
		ReadFile(hFile,&m_Count,4,&dw,NULL);
		if (m_Count)
		{
			beg=malloc(sizeof(T)*m_Count);
			ReadFile(hFile,beg,sizeof(T)*m_Count,&dw,NULL);
			byte sum=100;
			byte* t=(byte*)beg;
			for (int i=0; i<(int)(sizeof(T)*m_Count); i++)
			{
				byte newb=t[i]-sum;
				sum+=newb;
				t[i]=newb;
			}
		}
	}
	T& operator[] (int i) const
	{
		return *(T*)((int)beg+sizeof(T)*i);
	}
	DynArray()
		{ m_Count=0;	beg=NULL; };
};

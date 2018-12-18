class myC {
private:
		int a{0};
public:
		virtual ~myC()
		{
		}

		void *operator new(unsigned int i)
		{
// malloc(i);
			return (void*)0xdeadbeef;
		}

		void operator delete(void *p)
		{
// free(p);
		}
};

int main()
{
	myC c;
	myC* cc = new myC;
	delete cc;
}

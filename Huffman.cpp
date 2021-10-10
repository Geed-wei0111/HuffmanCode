#include <iostream>
#include <ctime>
#include <fstream>
#include <cstring>

//哈夫曼树的结点
struct BiNode
{
	char word;
	int times;
	BiNode* lchild;
	BiNode* rchild;
};

//队列的节点
struct Node
{
	BiNode* data;
	struct Node* next;
};

//哈夫曼表的节点（单链表）
struct TableNode
{
	char data;
	char codes[32];
	TableNode* next;
};

//十进制转二进制，用于解码
char* DECtoBIN(int number, int state)
{
	//将“负数”变成正数
	if (number <= 0)
		number = 256 + number;
	static char binary[9] = "";

	//如果state等于0表示还没到最后几位
	//如果不等于0表示最后几位不满8位需要特殊处理
	//最后几位即state位
	if (state == 0)
	{
		int times = 0;
		int temp = 0;
		int i = 7;
		while (times != 8)
		{
			temp = number % 2;
			number = number / 2;
			binary[i--] = '0' + temp;
			times++;
		}
		binary[8] = '\0';
	}
	else
	{
		int i = 0;
		int count = state;
		int temp = number;
		for (i = count - 1; i >= 0; i--)
		{
			temp = number % 2;
			binary[i] = '0' + temp;
			number = number / 2;
		}
		binary[count] = '\0';
	}
	return binary;
}

//文件控制类，用来管理数据的进出
class File
{
public:
	File()
	{
		for (int i = 0; i < 40960; i++)
		{
			article[i] = '\0';
			codes[i] = '\0';
		}
	}

	//将编码好的文章输出，将8位0或1拼接成1个字节再写入
	//文件开头的第一个字节用来存储最后几位的位数
	void OutputCodes(const char* encodes)
	{
		out.open("Code.Huffman", std::ios::binary);
		int num = 0;
		int i = 0, count = 0, k = 1;

		char* output = new char[40960];
		for (int i = 0; i < 40960; i++)
		{
			output[i] = '\0';
		}
		output[0] = '*';

		while (encodes[i] != '\0')
		{
			if (encodes[i] == '0')
				num = num << 1;
			else if (encodes[i] == '1')
			{
				num = num << 1;
				num += 1;
			}
			i++;
			count++;
			if (count == 8)
			{
				output[k++] = (char)num;
				num = count = 0;
			}
		}

		output[k++] = (char)num; 	//防止最后几位未写入文件
		output[k] = '\0';

		output[0] = char(count);
		out.write(output, sizeof(char) * k);
		out.close();
		delete[] output;
	}
	//导入文章
	void InputArticle()
	{
		char* buffer = new char[2048];
		for (int i = 0; i < 2048; i++)
		{
			buffer[i] = '\0';
		}
		in.open("source.txt");
		while (!in.eof())
		{
			in.getline(buffer, 2048);
			int len = strlen(buffer);
			buffer[len++] = char(10);//换行的ASCII码是10
			buffer[len] = '\0';

			strcat_s(article, sizeof(char) * 40960, buffer);
		}

		int len = strlen(article);
		article[len - 1] = '\0';
		in.close();
		delete[] buffer;
	}
	//把文章以指针形式传出去
	char* GetArticle()
	{
		return article;
	}
	//导入编码
	int InputCodes()
	{
		int i = 0;
		in2.open("Code.Huffman", std::ios::binary);

		static char end;
		in2.read(&end, sizeof(char));

		while (!in2.eof())
		{
			in2.read(&codes[i], sizeof(char));
			i++;
		}

		codeslen = i;
		in.close();
		return (int)end;
	}
	//把编码以指针形式传出去
	char* GetCodes()
	{
		return codes;
	}
	//获取编码总长度
	int GetCodesLen() { return codeslen; }
	//导出解码好的文章
	void OuputArticle(const char* art)
	{
		out2.open("NEW.txt", std::ios::binary);
		out2.write(art, sizeof(char) * strlen(art));
		out2.close();
	}
private:
	std::ofstream out, out2;
	std::ifstream in, in2;
	char article[40960];
	char codes[40960];
	int codeslen;
};

//优先队列类，比队列灵活
class Priority_queue
{
public:
	Priority_queue()
	{
		rear = front = new Node;
		front->next = nullptr;
	}
	~Priority_queue()
	{
		Node* p = this->front;
		Node* q = nullptr;

		while (p)
		{
			q = p->next;
			delete p;
			p = q;
		}
	}
	//入队操作
	void push(BiNode* t)
	{
		Node* neo = new Node;
		neo->data = t;
		if (front->next == nullptr)
		{
			front->next = neo;
		}
		else
		{
			rear->next = neo;
		}
		rear = neo;
		neo->next = nullptr;
	}
	//优先入队操作
	//正常入队是直接加在末尾，而优先入队是按从小到大或从大到小 插入 到队列里
	void priority_push(BiNode* t)
	{
		Node* p = this->front;
		while (p->next != nullptr)
		{
			if (p->next->data->times < t->times)
				p = p->next;
			else
				break;
		}
		Node* neo = new Node;
		neo->data = t;
		neo->next = p->next;
		p->next = neo;
	}
	//出队操作
	void pop()
	{
		Node* temp = front->next;
		if (front->next != nullptr)
		{
			if (front->next == rear)
			{
				delete rear;
				rear = front;
				front->next = nullptr;
			}
			else
			{
				front->next = temp->next;
				delete temp;
			}
		}
	}
	//获取队头
	BiNode* get_top()
	{
		if (front->next != nullptr)
			return front->next->data;
		else
			return nullptr;
	}
	//获取队列里的元素个数
	int count()
	{
		int count = 0;
		Node* p = this->front->next;
		while (p)
		{
			p = p->next;
			count++;
		}
		return count;
	}
	//查看队列是否为空
	int empty()
	{
		if (front->next)
			return 0;
		else
			return 1;
	}
private:
	Node* front, * rear;
};

//哈夫曼表，就是个有尾指针的单链表
//在创建完树之后，为了编码方便，把每个字符的码用单链表串起来
class Table
{
public:
	friend class HuffmanTree;
	Table()
	{
		first = last = new TableNode;
		first->next = nullptr;
	}
	~Table()
	{
		TableNode* p = nullptr;
		while (first)
		{
			p = first->next;
			delete first;
			first = p;
		}
	}
private:
	TableNode* first;
	TableNode* last;
};

//哈夫曼树
class HuffmanTree
{
public:
	friend class HuffmanTable;
	HuffmanTree()
	{
		root = nullptr;
		Decodes = new char[40960];
		test = new char[40960];
		for (int i = 0; i < 40960; i++)
		{
			Decodes[i] = '\0';
			test[i] = '\0';
		}
	}
	~HuffmanTree() { Release(root); delete[] Decodes, test; }


	//建造哈夫曼表
	void BuildTable()
	{
		int k = 0;
		char code[256];
		Traverse(root, code, k);
	}
	//创建完哈夫曼树时，给叶子结点存编码
	void Traverse(BiNode* bt, char code[256], int k)
	{
		if (bt->lchild == nullptr && bt->rchild == nullptr)
		{
			code[k] = '\0';
			TableNode* neo = new TableNode;
			strcpy_s(neo->codes, code);
			neo->data = bt->word;
			if (table.first->next == nullptr)
			{
				table.first->next = neo;
				table.last = neo;
			}
			else
			{
				table.last->next = neo;
				table.last = neo;
			}
			neo->next = nullptr;
		}
		if (bt->lchild)
		{
			code[k] = '0';
			Traverse(bt->lchild, code, k + 1);
		}
		if (bt->rchild)
		{
			code[k] = '1';
			Traverse(bt->rchild, code, k + 1);
		}
	}

	//编码
	void Encode(const char* str)
	{
		TableNode* p = nullptr;
		for (int i = 0; str[i] != '\0'; i++)
		{
			p = table.first;
			while (p && p->data != str[i])
				p = p->next;
			if (p)
				strcat_s(Decodes, sizeof(char) * 40960, p->codes);
		}
		std::cout << Decodes << std::endl;
	}
	//解码
	void Decode()
	{
		int k = 0;
		BiNode* p = root;
		for (int i = 0; Decodes[i] != '\0'; i++)
		{
			if (Decodes[i] == '0')
				p = p->lchild;
			else if (Decodes[i] == '1')
				p = p->rchild;

			if (p && p->lchild == nullptr && p->rchild == nullptr)
			{
				test[k++] = p->word;
				p = root;
			}
		}
		if (p && p->lchild == nullptr && p->rchild == nullptr)
			test[k++] = p->word;
		test[k] = '\0';
		std::cout << std::endl << test << std::endl;
		std::cout << "after len=" << strlen(test) << std::endl;
	}

	//释放结点
	void Release(BiNode* bt)
	{
		if (bt)
		{
			Release(bt->lchild);
			Release(bt->rchild);
			delete bt;
		}
	}

	//输出编码到文件
	void Output()
	{
		file.OutputCodes(Decodes);
		memset(Decodes, 0, sizeof(Decodes));
	}
	//将编码文件输入到程序里
	void Input()
	{
		char* neo = nullptr;
		End = file.InputCodes();

		neo = file.GetCodes();
		CodesLen = file.GetCodesLen();

		char* buffer = nullptr;
		int i = 0, count = 0, k = 0;
		strcpy_s(Decodes, sizeof(char) * 40960, "\0");
		while (i < CodesLen - 1)
		{
			if (i < CodesLen - 2)
				buffer = DECtoBIN(int(neo[i++]), 0);
			else
				buffer = DECtoBIN(int(neo[i++]), End);
			strcat_s(Decodes, sizeof(char) * 40960, buffer);
		}
		this->Decode();
		this->file.OuputArticle(test);
	}

	//根节点
	BiNode* root;

	//子对象，哈夫曼表
	Table table;

	//存储编码
	char* Decodes;
	//存储文章
	char* test;
	//子对象，文件操作，用于文本输入输出
	File file;

	//用来标记编码最后一个字节有几位
	int End;
	//用来标记编码一共有N个字节
	int CodesLen;
};

int main()
{
	HuffmanTree tree;
	//载入文件里的文章
	tree.file.InputArticle();
	//input指针指向文章
	char* input = tree.file.GetArticle();

	//统计文章中字符的出现次数（频率）
	int i;
	int* rate = new int[128];
	for (i = 0; i < 128; i++)
	{
		rate[i] = 0;
	}
	for (i = 0; input[i] != '\0'; i++)
	{
		rate[input[i]]++;
	}

	//开始创建哈夫曼树

	//先把已有的字母“变成”结点
	BiNode* temp = nullptr;
	Priority_queue q;
	for (i = 0; i < 128; i++)
	{
		if (rate[i] != 0)
		{
			temp = new BiNode;
			temp->lchild = temp->rchild = nullptr;
			temp->word = i;
			temp->times = rate[i];
			q.priority_push(temp);
		}
	}
	delete[] rate;

	//每次取出两个合成一个新结点，重复直到只剩一个
	//最后一个就是根节点
	BiNode* r = nullptr;
	BiNode* s = nullptr;
	BiNode* neo = nullptr;
	while (q.count() != 1)
	{
		r = q.get_top();
		q.pop();
		s = q.get_top();
		q.pop();

		neo = new BiNode;
		neo->word = '*';
		neo->times = r->times + s->times;
		neo->lchild = r;
		neo->rchild = s;

		q.priority_push(neo);
	}

	tree.root = q.get_top();
	q.pop();

	//创建哈夫曼表，用来直接翻译文章
	tree.BuildTable();
	//根据文章编码
	tree.Encode(input);
	//输出二进制文章编码
	tree.Output();
	//引入二进制文件编码并解码
	tree.Input();
	//原本文章的长度
	std::cout << "original len=" << strlen(input) << std::endl;
	return 0;
}
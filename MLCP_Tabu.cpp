// MLCP_Tabu.cpp : 定义控制台应用程序的入口点。
//复现Tabu论文

#include "stdafx.h"
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <sstream>
#include <fstream>
#include <time.h>
#include <ctime>
#include <string>
#include <string.h>
#include <algorithm>
#include <vector>

//该版本加入了swap操作，tt固定15，alpha0.01，omega1000，与exp相比omega变了，tt变了
using namespace std;
#define MAX_VAL 999999

clock_t startTime, endTime;//记录运行时间
double bestTime;//最佳时间
double cutting_time = 180;//终止条件
//string inFile = "C:/Users/18367/Documents/Visual Studio 2015/Projects/MLCP/Debug/data/le450_25a.col";
string inFile = "C:/Users/18367/Documents/Visual Studio 2015/Projects/MLCP/Debug/data/";
string outFile = "F:/科研文献/论文1/Tabu/180/";

vector<string> fileList = { 
	"myciel6.col",
	"homer.col",
	"mulsol.i.5.col",
	"inithx.i.1.col",
	"inithx.i.2.col",
	"inithx.i.3.col",
	"latin_square_10.col",
	"le450_5a.col",
	"le450_5b.col",
	"le450_5c.col",
	"le450_5d.col",
	"le450_15a.col",
	"le450_15b.col",
	"le450_15c.col",
	"le450_15d.col",
	"le450_25a.col",
	"le450_25b.col",
	"le450_25c.col",
	"le450_25d.col"
};

//主要调节的参数
int timeFlag = 0;
int tt = 2;//禁忌长度
int omega = 500;//扰动的阈值
float alpha = 0.01;//扰动强度
int noimprove = 0;//连续未更新的次数

int Sc;
int Sb;

int edgeNum, verNum;
int** edge;//两个飞机时间是否有冲突，有则1，无则0
int** adj;//记录跟每架飞机有冲突的飞机
int* adjlen;//记录跟每架飞机冲突飞机的数量

			//主要数据结构
int** color;
int* colorlen;
int* v2color;
int* v2idx;
int* conect;//与所在颜色的边
int* bian;//两个颜色中边的数量

		  //局部搜索
int* tabu;//禁忌表
int iter = 0;//迭代次数

			 //函数定义
int random_int(int);
void read_initial(string);
void random_initial();//随机构建初始解
void compute_obj();//计算目标值
void FLIP(int n);//将飞机flt从当前gate移动到gt
void show_result();
void local_search();
void judge_best();
void pertubation();//扰动

int restart;

int Sblist[10];
double timelist[10];


//随机生成一个 0-(n-1) 的整数
int random_int(int n)
{
	return rand() % n;
}

//读取数据
void read_initial(string inFile)
{
	int i, j, k;
	ifstream FIC;
	FIC.open(inFile);//读取数据
	if (FIC.fail())
	{
		cout << "### Erreur open, File_Name " << inFile << endl;
		getchar();
		exit(0);
	}
	if (FIC.eof())
	{
		cout << "### Error open, File_Name " << inFile << endl;
		getchar();
		exit(0);
	}
	FIC >> verNum >> edgeNum;
	edge = new int*[verNum];
	adj = new int*[verNum];
	color = new int*[2];
	adjlen = new int[verNum];
	tabu = new int[verNum];
	v2color = new int[verNum];
	v2idx = new int[verNum];
	conect = new int[verNum];
	colorlen = new int[2];
	bian = new int[2];
	colorlen[0] = 0;
	colorlen[1] = 0;
	bian[0] = 0;
	bian[1] = 1;
	color[0] = new int[verNum];
	color[1] = new int[verNum];

	for (i = 0; i < verNum; i++)
	{
		edge[i] = new int[verNum];
		adj[i] = new int[verNum];
		adjlen[i] = 0;
		tabu[i] = 0;
		conect[i] = 0;
	}
	for (i = 0; i < verNum; i++)
		for (j = 0; j < verNum; j++)
		{
			edge[i][j] = 0;
			adj[i][j] = 0;
		}

	int a, b;
	for (i = 0; i < edgeNum; i++)
	{
		FIC >> a >> b;
		a--;
		b--;
		edge[a][b] = edge[b][a] = 1;//如果二者有边相连

	}

	edgeNum = 0;
	for (i = 0; i < verNum - 1; i++)
		for (j = i + 1; j < verNum; j++)
			if (edge[i][j])
			{
				edgeNum++;
				adj[i][adjlen[i]] = j;
				adj[j][adjlen[j]] = i;
				adjlen[i]++;
				adjlen[j]++;
			}

	FIC.close();//关闭数据
}

//随机构建初始解
void random_initial()
{
	int i, j, k, m;
	colorlen[0] = 0;
	colorlen[1] = 0;
	bian[0] = 0;
	bian[1] = 1;
	for (i = 0; i < verNum; i++)
	{
		k = random_int(2);
		color[k][colorlen[k]] = i;
		v2color[i] = k;
		v2idx[i] = colorlen[k];
		colorlen[k]++;
		tabu[i] = 0;
		conect[i] = 0;
	}
	for (i = 0; i < verNum; i++)
		for (j = 0; j < verNum; j++)
			if (edge[i][j])
				if (v2color[i] == v2color[j])
					conect[i]++;
	for (i = 0; i < verNum; i++)
	{
		k = v2color[i];
		bian[k] += conect[i];
	}
	bian[0] /= 2;
	bian[1] /= 2;
	Sc = min(bian[0], bian[1]);
	Sb = Sc;
	restart = 0;
}

//局部搜索
void local_search()
{
	int delta;
	int temp_delta = -MAX_VAL;
	int swap_delta = -MAX_VAL;
	int i, j, k, m;
	int mark = -1;
	int swap_0 = -1;
	int swap_1 = -1;
	int org_0, org_1;
	//SWAPmove
	for (i = 0; i < colorlen[0]; i++)
	{
		org_0 = color[0][i];
		if (tabu[org_0] > iter)
			continue;
		for (j = 0; j < colorlen[1]; j++)
		{
			org_1 = color[1][j];
			if (tabu[org_1] > iter)
				continue;
			k = bian[1] + adjlen[org_0] - conect[org_0] - conect[org_1] - edge[org_0][org_1];
			m = bian[0] + adjlen[org_1] - conect[org_1] - conect[org_0] - edge[org_1][org_0];
			delta = min(k, m) - Sc;
			if (delta > swap_delta)
			{
				swap_delta = delta;
				swap_0 = org_0;
				swap_1 = org_1;
			}
		}
	}

	tabu[swap_0] = iter + tt;
	tabu[swap_1] = iter + tt;
	FLIP(swap_0);
	FLIP(swap_1);
	judge_best();

}


//翻转操作
void FLIP(int n)
{
	int org_color;
	int org_idx;
	int i, j, k, m;
	org_color = v2color[n];
	org_idx = v2idx[n];

	color[1 - org_color][colorlen[1 - org_color]] = n;
	v2color[n] = 1 - org_color;
	v2idx[n] = colorlen[1 - org_color];
	colorlen[1 - org_color]++;
	bian[org_color] -= conect[n];
	conect[n] = adjlen[n] - conect[n];
	bian[1 - org_color] += conect[n];

	colorlen[org_color]--;
	m = color[org_color][colorlen[org_color]];
	color[org_color][org_idx] = m;
	v2idx[m] = org_idx;

	//更新conect
	for (i = 0; i < adjlen[n]; i++)
	{
		k = adj[n][i];
		m = v2color[k];
		if (m == org_color)
			conect[k]--;
		else
			conect[k]++;
	}
	Sc = min(bian[0], bian[1]);
}

void judge_best()
{
	if (Sc > Sb)
	{
		Sb = Sc;
		noimprove = 0;
		endTime = clock();//计时
		bestTime = (double)(endTime - startTime) / CLOCKS_PER_SEC;
		//compute_obj();
		//cout << "iter: " << iter << " Sc: " << Sc << " time: " << bestTime << "s" << endl;
	}
	else
	{
		noimprove++;
		if (noimprove > omega)
		{
			/*noimprove = 0;
			pertubation();*/
			//restart = 1;
		}
	}
}


void show_result()
{
	int i, j, k, m;

	cout << "颜色0[" << colorlen[0] << "]bian0[" << bian[0] << "]: ";
	for (i = 0; i < colorlen[0]; i++)
	{
		cout << color[0][i] + 1 << "[" << conect[color[0][i]] << "] ";
	}
	cout << endl << "颜色1[" << colorlen[1] << "]bian1[" << bian[1] << "]: ";
	for (i = 0; i < colorlen[1]; i++)
	{
		cout << color[1][i] + 1 << "[" << conect[color[1][i]] << "] ";
	}
	cout << endl;

}


void pertubation()//扰动
{
	int i, j, k;

	k = max(int(alpha * verNum), 1);
	//cout <<"iter: "<<iter<< " 扰动前，sc: " << Sc << endl;
	for (i = 0; i < alpha * verNum; i++)
	{
		j = random_int(verNum);
		FLIP(j);
		//tabu[j] = iter + 0.2 * tt;
		tabu[j] = 0;
	}
	//cout << "iter: " << iter<< "扰动后，sc: " << Sc << endl;
}

//主函数
int main(int argc, char** argv)
{
	/*if (argc == 3)
	{
	inFile = argv[1];
	cutting_time = atoi(argv[2]);
	}
	else
	{
	cout << "error: MLCPexp inFile time" << endl;
	getchar();
	exit(0);
	}*/
	srand((unsigned)time(NULL));
	for (int ll = 0; ll < 19; ll++) {

		string fileName = inFile + fileList[ll];
		string outputFileName = outFile + fileList[ll];
		ofstream outFile(outputFileName, ios::app);

		int t = 0;
		int max = 0;
		double tt = 0;
		
		read_initial(fileName);//读取数据
		//cout << inFile << endl << endl;
		//tt = min((int)(0.2 * verNum), 20);
		//cout << "tt: " << tt << endl;
		for (int i = 0; i < 1; i++) {
			timeFlag = 10;
			startTime = clock();//计时开始	
			endTime = startTime;
			random_initial();
			//cout << "Sc: " << Sc << endl;
			while ((endTime - startTime) / CLOCKS_PER_SEC < cutting_time)
			{
				//show_result();
				iter++;
				local_search();
				endTime = clock();//计时
				if (restart == 1) {
					break;
				}
			}
			Sblist[i] = Sb;
			timelist[i] = bestTime;
			outFile << verNum << " " << edgeNum << " " << Sb << " " << bestTime << endl;
		}
	}

	
}



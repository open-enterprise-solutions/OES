#if !defined(_MOXCEL_H__)
#define _MOXCEL_H__

#include <wx/wx.h>
#include <wx/file.h>

#define MOXEL_HEADER wxString("MOXCEL")
#define MOXEL_HEADER_LENGTH MOXEL_HEADER.Length()

#define MY_MOXEL_HEADER wxString("MYMXL2")
#define MY_MOXEL_HEADER_LENGTH MY_MOXEL_HEADER.Length()

#include "grid/grid_sections.h"

//����������� �����
void write(wxFile &f, wxString &Str);
void read(wxFile &f, wxString &Str);

class CCell
{
public:
	int nColumn;		//����� �������
	int nRow;

	byte nMode;//����� (0-�����,1-���������,2-������...)
	byte nType;		//��� ������ ������ U(��������������),N(�����),S(������),B(����������),O(��������),T(����),K(��� ��������)
	byte nLen;		//�����
	byte nPrec;		//��������
	bool bProtect;	//���� ������ ������
	byte nControl;	//�������� (0-����,1-��������,2-��������...)
	byte nHPosition;	//��������� �� ����������� (0-����,1-�����,2-�� ������,3-�����) ������� �������� ����� - ���������� ��������, �� �������� ������������� ������
	byte nVPosition;	//��������� �� ��������� (0-����,1-���,2-�����)
	
	//�����:
	COLORREF nRamkaColor;//���� �����

	byte nRamkaL;//�����
	byte nRamkaU;//������
	byte nRamkaR;//������
	byte nRamkaD;//�����
	
	//�������� - �����:
	//1-�����1
	//2-������ �����
	//3-������� �����
	//4-����� ������� �����
	//5-������� �����
	//6-�������1
	//7-�������2
	//8-�����2
	//9-������� �������

	COLORREF nBackgroundColor;//���� ���� (0-55 �� �������)
	byte nUzor;//���� (0-15)
	COLORREF nUzorColor;//���� ����� (0-55 �� �������)

	__int16 nFontNumber;//����� ������ �� ������
	bool nBold;//��������
	bool nItalic;//�����������
	bool nUnderLine;//�������������
	COLORREF nFontColor;//���� ������
	int nFontHeight;//������ ������

	__int16 nCoveredRow;
	__int16 nCoveredCol;

	wxString csText;		//����� ������
	wxString csFormula;	//�����������

	wxString csFormat;
	wxString csMaska;

	CCell()
	{
		nMode = 0;
		nColumn = 0;
		nRow = 0;
		nType = 0;
		nLen = 0;
		nPrec = 0;
		bProtect = 1;
		nControl = 0;
		nHPosition = 0;
		nVPosition = 0;

		nRamkaColor = 0;
		nRamkaL = 0;
		nRamkaU = 0;
		nRamkaR = 0;
		nRamkaD = 0;
		nBackgroundColor = 0xFFFFFF;
		nUzor = 0;
		nUzorColor = 0;

		nFontNumber = 0;
		nBold = 0;
		nItalic = 0;
		nUnderLine = 0;
		nFontColor = 0;
		nFontHeight = 0;
		nCoveredRow = 0;
		nCoveredCol = 0;
	};

	//�����������
	void Load(wxFile &f);
	void Save(wxFile &f);
};

class CMoxcel;

class CRow
{
public:
	CRow();
	virtual ~CRow();
public:
	int nColumnCount;//����� ����������� �������
	CCell *aCell;
	int nRowHeight;//������ ������
public:

	//�������� 1� �������
	void Load(char *buf, int &nPos);

	//�����������
	void Load(wxFile &sf);
	void Save(wxFile &sf);
};

class CMoxcel
{
public:

	CMoxcel();
	virtual ~CMoxcel();
	void Clear();

public:
	int nAllColumnCount;//����� ������� � �������
	int nAllRowCount;//����� ����� � �������

	int nRowCount;//����� ����������� �����
	int *aRowNumber;//������ �����
	CRow *aRow;//������

	int nWidthCount;//������ ������� ������� ������ �������
	int *aWidthNumber;//������ ������� � ������� �������� ������
	int *aWidth;//������ �������

	int nFontCount;//���������� �������
	int *aFontNumber;//������ �������
	wxString *aFont;//������

	int nMergeCells;//���������� ������������ �����
	int *aMergeCells;//������ ������������ �����

	//������:
	CSectionArray HorizSection;
	CSectionArray VertSection;

public:

	//�������� 1� �������
	bool Load(wxString csBuffer);
	bool Save(wxString &csBuffer);

	//�����������
	void Load(wxFile &sf);
	void Save(wxFile &sf);

protected:

	void Load(char *buf);
};

//��� ������� �� ������ � 1� ��������
//UINT GetPallete(int nIndex);
int GetRamka(int nIndex);
int GetUzor(int nIndex);

#endif // !defined(_MOXCEL_H__)

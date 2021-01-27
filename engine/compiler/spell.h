#ifndef __SPELL_H__
#define __SPELL_H__

#include "compiler.h"

enum SpellPatrimony_ 
{
	patrMan = 0, 
	patrFem = 1,
	patrInd = 2
}; //–од ("наследственность")

struct SpellPatrimony 
{
	short patr;

	SpellPatrimony();
	SpellPatrimony(short c);

	operator short();
};


class spell_string 
{
protected:
	wxString str; //"Ќачальна€ форма"
public:
	spell_string();
	spell_string(spell_string&);
	spell_string(wxString);
	virtual wxString& operator[](int);
	virtual wxString& operator[](double);
	virtual SpellPatrimony operator =(int);
	virtual operator SpellPatrimony();
	wxString operator=(wxString);
	spell_string& operator=(spell_string&);
	inline operator wxString&() { return str; };
	friend class spell_num;
};

class spell_unit : public spell_string {
protected:
	//Ќач. форма - ед. число
	wxString unit2, unit5;
	SpellPatrimony patr;
public:
	spell_unit();
	spell_unit(spell_unit&);
	spell_unit(wxString, int = patrMan);
	spell_unit(wxString, wxString, wxString, int = patrMan);
	virtual wxString& operator[] (int n);
	virtual wxString& operator[] (double d);
	virtual SpellPatrimony operator =(int);
	friend class spell_num;
};

class spell_patr : public spell_string 
{

protected:
	// Ќач. форма - мужск. род
	wxString sFem, sInd;
public:
	
	spell_patr();
	spell_patr(spell_patr&);
	spell_patr(wxString);
	spell_patr(wxString, wxString, wxString);
	
	virtual wxString& operator[] (int n);
	virtual wxString& operator[] (double d);

	friend class spell_num;
};

class spell_num 
{

	wxString	plus, minus;
	spell_string	num0;
	spell_patr num1_2[2];
	spell_string num3_19[19 - 3 + 1];
	spell_string num20_90[(90 - 20) / 10 + 1];
	spell_string num100_900[(900 - 100) / 100 + 1];
	spell_unit numexp[5]; //10^0 - всегда пустое

	int Decode999(wxString &rez, int arg, int patr);

public:
	
	spell_num();
	spell_num(spell_num&);
	
	spell_string& operator[] (int n);
	spell_string& operator[] (double d);

	wxString operator() (int n, int patr = patrMan);
	wxString operator() (double d, int patr = patrMan);

	wxString operator() (int n, spell_unit &su);
	wxString operator() (double d, spell_unit &su);
	wxString operator() (double d, spell_unit &su, spell_unit &fu);

	void SetSpell(wxString[]);
	static void SetDefaultSpell(spell_num&);
};

class spell_date
{

public:

	wxString Months_And_Other[24]
		//12мес + 12мес в р.п.,
		, Quart, SemiYear, Month_9, Year, SmallYear;
	//+ кв + 1/2г + 9мес + год + сокр. год. =29 - и все идет подр€д
	spell_date();
	wxString DayOfWeek[7];
	enum Interval {
		InM = 12, InQ = 25, InS = 26, InMonths_9 = 27
		, InYear = 28, InSmallYear = 29
	};
	wxString& operator[] (int n);
	wxString operator() (int n);
};

struct Speller 
{
	Speller();
	Speller(const Speller& sp);
	
	spell_unit *UnitBig, *UnitSmall;
	spell_date DatePart;
	spell_num num_Speller;
	static Speller& GetDefaultSpeller();
	void SetAsDefaultSpeller();
	void CopyToDefaultSpeller();
	
	~Speller();

	static wxString FloatFormat(double d, int width, int prec, int leadzero, char zero, char dot, char coma);
	static void FirstCharBig(wxString&); //устанавливает 1-ю букву строки большой, остальные-маленькие
	static wxString GetFract(double d, int prec);
};

#endif __SPELL_H__

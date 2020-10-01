#include <iostream>
#include <sstream>
#include <stack>
#include <memory>
#include <map>
#include <cmath>
#include <stdexcept>
#include <functional>

enum enPriority
{
	High,
	Medium,
	low,

	unknow
};

/*
Map structure:
1) The priority of the operation '()' Highest; '* / ^' Middle, and etc
2) Calculation Function
*/
struct sForm
{
	enPriority Prioritet;
	std::function<double(double a, double b)> Formula;
};

// --------------------------------------------------------------
class CCalc
{
public:
	CCalc()
	{}

	explicit CCalc(const std::string &str)
		: sFormula(str), 
		bKeyOpen(false), 
		lFrstSym(0), 
		itr(0)
	{
		InitMap();
	}

	~CCalc()
	{}

	void ParseString();
	std::string GetAnswer() const;

private:
	void InitMap();
	void CallParse(long _l, long _r);
	double CalcStr(std::string _formula);
	bool CheckPrior();
	bool CheckLvl(long lcurLvl);
	void CheckPriorBrackets();
	void ResetCursor();

	bool bKeyOpen;
	long lFrstSym;
	long itr;

	std::string sFormula;
	std::map<char, sForm> mFrml;
	std::stack<double> stValue;
	std::stack<char*> stSign;
};

// --------------------------------------------------------------
void CCalc::InitMap()
{
	sForm sF;

	sF.Prioritet = enPriority::low;
	sF.Formula = [](double a, double b) { return a + b; };
	mFrml.insert(std::make_pair('+', sF));

	sF.Formula = [](double a, double b) { return a - b; };
	mFrml.insert(std::make_pair('-', sF));

	sF.Prioritet = enPriority::Medium;
	sF.Formula = [](double a, double b) { return a * b; };
	mFrml.insert(std::make_pair('*', sF));

	sF.Formula = [](double a, double b) { return a / b; };
	mFrml.insert(std::make_pair('/', sF));

	sF.Formula = [](double a, double b) { return fmod(a, b); };
	mFrml.insert(std::make_pair('%', sF));

	sF.Formula = [](double a, double b) { return pow(a, b); };
	mFrml.insert(std::make_pair('^', sF));

	sF.Prioritet = enPriority::High;
	sF.Formula = NULL;
	mFrml.insert(std::make_pair('(', sF));
	mFrml.insert(std::make_pair(')', sF));
}

// --------------------------------------------------------------
void CCalc::ResetCursor()
{
	lFrstSym = 0;
	itr = 0;
}

// --------------------------------------------------------------
double CCalc::CalcStr(std::string _formula)
{
	auto PopStack([&]() {
		if (!stValue.empty())
		{
			auto r(stValue.top());
			stValue.pop();
			return r;
		}
		return 0.0;
	});

	double res(0.0);
	while (!_formula.empty())
	{
		double val(0.0);
		std::stringstream ss(_formula);
		if (ss >> val && (_formula.front() != '+' && _formula.front() != '-')) // Checking for a value, excluding the sign
		{
			stValue.push(val);
			if (stValue.size() > 1)
			{
				auto _r{PopStack()};
				auto _l{PopStack()};
				try
				{
					const auto &op(mFrml.at(*stSign.top()));
					res = {op.Formula(_l, _r)};
					stValue.push(res);
					stSign.pop();
				}
				catch (const std::out_of_range &)
				{
					throw std::invalid_argument(&_formula.front());
				}
			}

			_formula.erase(0, ss.tellg());
		}
		else
		{
			if (_formula.front() != ' ')
				stSign.push((char *)(_formula.substr(0, 1).c_str()));

			_formula.erase(0, 1);
		}
	}
	if (!stValue.empty())
		stValue.pop();

	return res;
}

// --------------------------------------------------------------
// Get from the main formula, necessary substring
// Computed value (CalcStr)
// Insert back the main string to count
// --------------------------------------------------------------
void CCalc::CallParse(long _l, long _r)
{
	long diff(_r - _l);
	if (diff > 0)
	{
		double cout = CalcStr(std::string(sFormula.substr(_l, diff)));
		sFormula.erase(_l, diff);
		sFormula.insert(_l, std::to_string(cout));
	}
}

// --------------------------------------------------------------
//  Function for checking the priority of characters in the required formula
// --------------------------------------------------------------
bool CCalc::CheckPrior()
{
	if (bKeyOpen)
	{
		CallParse(lFrstSym, itr - 1);

		ResetCursor();
		bKeyOpen = false;
	}
	else
		bKeyOpen = true;

	return bKeyOpen;
}

// --------------------------------------------------------------
// Checking the current priority
// If there are no more characters, lower the priority
// --------------------------------------------------------------
bool CCalc::CheckLvl(long lcurLvl)
{
	bool bflag(false);
	for (auto &it : mFrml)
	{
		if (lcurLvl == it.second.Prioritet && (std::string::npos != sFormula.find(it.first)))
		{
			bflag = true;
			break;
		}
	}
	return bflag;
}

// --------------------------------------------------------------
// A breakdown of the priority of inside the brackets
// --------------------------------------------------------------
void CCalc::CheckPriorBrackets()
{
	long lvl(-1);
	long needcur(-1);
	bool _flag(false);

	for (long i=lFrstSym; i < itr; ++i)
	{
		auto fSym = mFrml.find(sFormula[i]);
		if (fSym != mFrml.end())
		{
			if (lvl == -1 && fSym->second.Formula != NULL)
			{
				lvl = fSym->second.Prioritet;
				needcur = i;
			}
			else if (fSym->second.Formula != NULL && lvl > fSym->second.Prioritet)
			{
				_flag = true;
				break;
			}
			else if (fSym->second.Formula != NULL)
				needcur = i;
		}
	}

	if (needcur !=-1 && _flag)
		CallParse(needcur+1, itr);
	else
		CallParse(lFrstSym, itr + 1);

	ResetCursor();
}

// --------------------------------------------------------------
void CCalc::ParseString()
{
	bool bOpenBracket(false);
	long lcurLvl(0);

	while (true)
	{
		auto _mSym = mFrml.find(sFormula[itr]);
		if (_mSym != mFrml.end())
		{
			switch (_mSym->second.Prioritet)
			{
			case enPriority::High:
				if (sFormula[itr] == '(')
				{
					lFrstSym = itr;
					bOpenBracket = true;
				}
				else if

					(sFormula[itr] == ')' && bOpenBracket)
				{
					CheckPriorBrackets();
					bOpenBracket = false;
				}
				break;
			case enPriority::Medium:
			case enPriority::low:
				if (!bOpenBracket)
				{
					if (_mSym->second.Prioritet == lcurLvl)
					{
						if (!CheckPrior())
							continue;
					}
					else
						lFrstSym = itr + 1;
				}
				break;
			default:
				break;
			}
		}

		if (itr >= sFormula.size())
		{
			if (bKeyOpen)
			{
				CallParse(lFrstSym, itr);
				bKeyOpen = false;
			}

			if (!CheckLvl(lcurLvl))
				++lcurLvl;

			if (lcurLvl == enPriority::unknow)
				return;

			ResetCursor();
		}
		else
			++itr;
	}
}

// --------------------------------------------------------------
std::string CCalc::GetAnswer() const
{
	std::string res = sFormula.substr(0, sFormula.find_last_not_of('0') + 1);
	if (res.back() == '.')
		res.pop_back();

	return res;
}

// --------------------------------------------------------------
int main(void)
{
	std::string str;
	std::cin >> str;
	std::unique_ptr<CCalc> Calc = std::make_unique<CCalc>(str);
	Calc.get()->ParseString();

	if (str.compare( Calc.get()->GetAnswer()) != 0)
		std::cout <<"Your Answer: "<< Calc.get()->GetAnswer();
	else 
		std::cout <<"Sorry(";

	return 0;
}
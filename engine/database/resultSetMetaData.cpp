#include "resultSetMetaData.h"

inline bool CompareString(const wxString &lhs, const wxString &rhs)
{
	return lhs.CompareTo(rhs, wxString::ignoreCase) == 0;
}

int ResultSetMetaData::FindColumnByName(const wxString &colName)
{
	for (int i = 1; i <= GetColumnCount(); i++)
	{
		if (CompareString(colName, GetColumnName(i)))
			return i;
	}

	return wxNOT_FOUND;
}

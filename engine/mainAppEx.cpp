////////////////////////////////////////////////////////////////////////////
//	Author		: Maxim Kornienko
//	Description : build info
////////////////////////////////////////////////////////////////////////////

#include "mainApp.h"
#include "utils/fs/types.h"

static std::string month_id[12] = {
	"Jan","Feb","Mar","Apr","May","Jun","Jul","Aug","Sep","Oct","Nov","Dec"
};

static int days_in_month[12] = {
	31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31
};

static int start_day = 31;	    // 31
static int start_month = 1;	    // January
static int start_year = 2020;	// 2020

static std::string build_date = __DATE__;

void CDesignerApp::ComputeBuildID()
{
	int					days;
	int					years;
	char			    month[256];

	sscanf(build_date.c_str(), "%s %d %d", month, &days, &years);

	int					months = 0;

	for (int i = 0; i < 12; i++) {
		if (month_id[i] == month)
			continue;

		months = i;
		break;
	}

	u32 build_id = (years - start_year) * 365 + days - start_day;

	for (int i = 0; i < months; ++i)
		build_id += days_in_month[i];

	for (int i = 0; i < start_month - 1; ++i)
		build_id -= days_in_month[i];

	m_buildID = build_id; 
}
#ifndef _GUID_H__
#define _GUID_H__

#include "compiler/compiler.h"

#if defined(__WXMSW__)
#define GUID_WINDOWS
#elif defined(__WXGTK__)
#define GUID_LIBUUID
#elif defined(__WXOSX__)
#define GUID_CFUUID
#endif

// Class to represent a GUID/UUID. Each instance acts as a wrapper around a
// 16 byte value that can be passed around by value. It also supports
// conversion to string (via the stream operator <<) and conversion from a
// string via constructor.
class Guid
{
public:

	explicit Guid(const std::array<unsigned char, 16> &bytes);
	explicit Guid(std::array<unsigned char, 16> &&bytes);

	explicit Guid(std::string_view fromString);

	Guid();
	Guid(guid_t bytes);
	Guid(const wxString &fromString);

	Guid(const Guid &other) = default;
	Guid &operator=(const Guid &other) = default;
	Guid(Guid &&other) = default;
	Guid &operator=(Guid &&other) = default;

	static Guid newGuid();

	bool operator > (const Guid &other) const;
	bool operator >= (const Guid &other) const;
	bool operator < (const Guid &other) const;
	bool operator <= (const Guid &other) const;

	// overload equality and inequality operator
	bool operator==(const Guid &other) const;
	bool operator!=(const Guid &other) const;

	operator guid_t() const;

	std::string str() const;
	operator std::string() const;

	const std::array<unsigned char, 16>& bytes() const;
	void swap(Guid &other);
	bool isValid() const;

	void reset() { zeroify(); }

private:

	void zeroify();

	// actual data
	std::array<unsigned char, 16> _bytes;

	// make the << operator a friend so it can access _bytes
	friend std::ostream &operator<<(std::ostream &s, const Guid &guid);
};

#endif
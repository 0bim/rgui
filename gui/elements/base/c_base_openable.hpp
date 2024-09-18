#pragma once
#ifndef C_BASE_OPENABLE_HPP
#define C_BASE_OPENABLE_HPP

class c_base_openable
{
public:
	c_base_openable() : m_is_open(false)
	{

	}
	
public:
	inline virtual void set_is_open(const bool open)
	{
		m_is_open = open;
	}

	inline bool get_is_open() const
	{
		return m_is_open;
	}

	inline virtual void toggle_is_open()
	{
		m_is_open ^= true;
    }

protected:
	bool m_is_open;
};

#endif
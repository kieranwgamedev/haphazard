/*
FILE: Any.inl
PRIMARY AUTHOR: Kieran Williams

Copyright (c) 2017 DigiPen (USA) Corporation.
*/
#pragma once

namespace meta
{

	template <typename T>
	Any::Any(const T& object) : m_type(GetTypePointer<T>())
	{
		if (sizeof(T) > MAX_SIZE)
		{
			m_dataPointer = new char[sizeof(T)];
			new (m_dataPointer) T(object);
			m_usesPointer = true;
		}
		else
		{
			// Copy the data into our data storage.
			new (m_data) T(object);
			m_usesPointer = false;
		}
	}

	template <typename T>
	T& Any::GetData()
	{
		assert(GetTypePointer<T>() == m_type);

		if (m_usesPointer)
		{
			return *reinterpret_cast<T *>(m_dataPointer);
		}
		else
		{
			return *reinterpret_cast<T *>(&m_data);
		}
	}

	template <typename T>
	void Any::SetData(const T& data)
	{
		assert(GetTypePointer<T>() == m_type);

		if (m_usesPointer)
		{
			*reinterpret_cast<T *>(m_dataPointer) = data;
		}
		else
		{
			*reinterpret_cast<T *>(&m_data) = data;
		}
	}

	//-----------------------
	// Member Getter/Setters
	//-----------------------

	template <typename T>
	void Any::SetMember(Member *member, const T& value)
	{
		assert(!m_type->IsPointerType());
		assert(member->GetType() == GetTypePointer<T>());

		member->Set(GetDataPointer(), &value);
	}

	template <typename T>
	void Any::SetMember(const char *memberName, const T& value)
	{
		assert(!m_type->IsPointerType());
		Member *member = m_type->GetMember(memberName);

		assert(member->GetType() == GetTypePointer<T>());

		member->Set(GetDataPointer(), &value);
	}

	template <typename T>
	T Any::GetMember(Member *member) const
	{
		assert(!m_type->IsPointerType());
		assert(member->GetType() == GetTypePointer<T>());

		return *reinterpret_cast<T *>(member->Get(GetDataPointer()));
	}

	template <typename T>
	T Any::GetMember(const char *memberName) const
	{
		assert(!m_type->IsPointerType());
		Member *member = m_type->GetMember(memberName);

		assert(member->GetType() == GetTypePointer<T>());

		return *reinterpret_cast<const T *>(member->Get(GetDataPointer()));
	}

	//-------------------------------
	// Pointer Member Getter/Setters
	//-------------------------------

	template <typename T>
	void Any::SetPointerMember(Member *member, const T& value)
	{
		assert(m_type->IsPointerType());
		assert(member->GetType() == GetTypePointer<T>());

		member->Set(GetDeepestDataPointer(), &value);
	}

	template <typename T>
	void Any::SetPointerMember(const char *memberName, const T& value)
	{
		assert(m_type->IsPointerType());
		Member *member = m_type->GetDeepestDereference()->GetMember(memberName);

		assert(member->GetType() == GetTypePointer<T>());

		member->Set(GetDeepestDataPointer(), &value);
	}

	template <typename T>
	T Any::GetPointerMember(Member *member) const
	{
		assert(m_type->IsPointerType());
		assert(member->GetType() == GetTypePointer<T>());

		return *reinterpret_cast<T *>(member->Get(GetDeepestDataPointer()));
	}

	template <typename T>
	T Any::GetPointerMember(const char *memberName) const
	{
		assert(m_type->IsPointerType());
		Member *member = m_type->GetDeepestDereference()->GetMember(memberName);

		assert(member->GetType() == GetTypePointer<T>());

		return *reinterpret_cast<T *>(member->Get(GetDeepestDataPointer()));
	}
}
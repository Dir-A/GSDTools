#pragma once
#include "SPT_HDR.h"
#include "SPT_Code.h"

#include <stdexcept>


namespace GSD::SPT
{
	template <typename SPT_OBJ_T>
	void CheckDumpDataMatched(uint8_t* pOrg, SPT_OBJ_T& rfOBj)
	{
		Rut::RxMem::Auto obj_mem = rfOBj.Make();
		if (memcmp(pOrg, obj_mem.GetPtr(), obj_mem.GetSize()))
		{
			throw std::runtime_error("Dump data mismatch");
		}
	}


	class Parser
	{
	private:
		HDR m_HDR;
		std::vector<Code> m_vcCode;

	public:
		Parser();

		void Load(std::wstring_view wsPath);
		void Load(uint8_t* pData);
		Rut::RxMem::Auto Make() const;
		Rut::RxJson::JValue Make(size_t nCodePage) const;

	public:
		size_t GetSize() const;
		std::vector<Code>& GetCodeList();
	};
}
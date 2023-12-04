#pragma once
#include <vector>
#include <string>
#include <memory>
#include <stdexcept>

#include "GSD_Types.h"
#include "../../Rut/RxStr.h"
#include "../../Rut/RxMem.h"
#include "../../Rut/RxJson.h"


namespace GSD::SPT
{
	template <typename SPT_OBJ_T>
	static bool CheckDump(uint8_t* pOrg, SPT_OBJ_T& rfOBj)
	{
		Rut::RxMem::Auto mem_data = rfOBj.Dump();
		return memcmp(pOrg, mem_data.GetPtr(), mem_data.GetSize()) ? false : true;
	}

	static std::wstring NumToHexWStr(const size_t nValue)
	{
		wchar_t buf[64];
		(void)swprintf_s(buf, 64, L"0x%08X", nValue);
		return buf;
	}

	static void DBCSReadChar(std::string& msText, uint32_t uiChar)
	{
		char dbcs_char_l = (char)((uiChar >> 0) & 0x00FF);
		char dbcs_char_h = (char)((uiChar >> 8) & 0x00FF);
		((uint8_t)dbcs_char_l >= 0x81u) ? (void)(msText.append(1, dbcs_char_l), msText.append(1, dbcs_char_h)) : (void)(msText.append(1, dbcs_char_l));
	}


	class SPT_Code_Parameter_Type0
	{
	private:
		uint32_t m_uiUn0 = 0;
		uint32_t m_uiUn1 = 0;
		uint32_t m_uiUn2 = 0;
		uint32_t m_uiUn3 = 0;
		uint32_t m_uiStrType0Len = 0; // dbcs char count
		uint32_t m_uiStrType1Len = 0;
		uint32_t m_uiStrType2Len = 0;

		std::vector<SPT_Char_Entry> m_vcCharList;
		std::string m_msStrType1;
		std::string m_msStrType2;

	private:
		size_t m_nMemSize = 0;

		void CountSize()
		{
			size_t size = 0;

			size += sizeof(this->m_uiUn0) + sizeof(this->m_uiUn1) + sizeof(this->m_uiUn2) + sizeof(this->m_uiUn3);
			size += sizeof(this->m_uiStrType0Len) + sizeof(this->m_uiStrType1Len) + sizeof(this->m_uiStrType2Len);
			size += this->m_vcCharList.size() * sizeof(SPT_Char_Entry);

			if (this->m_uiStrType1Len)
			{
				size += this->m_uiStrType1Len + 1;
			}
			if (this->m_uiStrType2Len)
			{
				size += this->m_uiStrType2Len + 1;
			}

			this->m_nMemSize = size;
		}

		std::wstring ReadText(size_t uiCodePage) const
		{
			std::vector<uint32_t> char_list;
			{
				for (auto& char_entry : this->m_vcCharList)
				{
					switch (char_entry.uiType)
					{
					case 0x7:// normal char flag
					{
						char_list.push_back(char_entry.uiChar);
					}
					break;

					case 0x8:// notation beg flag
					{
						char_list.insert(char_list.end() - char_entry.uiNotationCount, '<');
						char_list.push_back('/');
					}
					break;

					case 0x9: // notation end flag
					{
						char_list.push_back('>');
					}
					break;

					case 0xD: // string end flag
					{
						// Nothing
					}
					break;

					default:
					{
						throw std::runtime_error("Unknow Char Type!");
					}
					}
				}
			}

			std::string text;
			for (auto charx : char_list)
			{
				DBCSReadChar(text, charx);
			}

			return Rut::RxStr::ToWCS(text, uiCodePage);
		}

	public:
		SPT_Code_Parameter_Type0()
		{

		}

		void Parse(uint8_t* pData)
		{
			uint8_t* cur_prt = pData;

			{
				uint32_t* tmp_ptr = (uint32_t*)pData;
				this->m_uiUn0 = tmp_ptr[0];
				this->m_uiUn1 = tmp_ptr[1];
				this->m_uiUn2 = tmp_ptr[2];
				this->m_uiUn3 = tmp_ptr[3];
				this->m_uiStrType0Len = tmp_ptr[4];
				this->m_uiStrType1Len = tmp_ptr[5];
				this->m_uiStrType2Len = tmp_ptr[6];
			}
			cur_prt += 4 * 7;

			if (this->m_uiStrType0Len)
			{
				const SPT_Char_Entry* char_entry_arry = (SPT_Char_Entry*)cur_prt;
				for (size_t ite_entry = 0; ite_entry < this->m_uiStrType0Len; ite_entry++)
				{
					SPT_Char_Entry entry = char_entry_arry[ite_entry];
					m_vcCharList.push_back(entry);
				}

				cur_prt += m_vcCharList.size() * sizeof(SPT_Char_Entry);
			}
			if (this->m_uiStrType1Len)
			{
				this->m_msStrType1 = { (char*)cur_prt };
				cur_prt += this->m_uiStrType1Len + 1;
			}
			if (this->m_uiStrType2Len)
			{
				this->m_msStrType2 = { (char*)cur_prt };
				cur_prt += this->m_uiStrType1Len + 1;
			}

			this->CountSize();
		}

		Rut::RxMem::Auto Dump() const
		{
			Rut::RxMem::Auto mem_data;
			mem_data.SetSize(this->GetSize());

			uint8_t* cur_prt = mem_data.GetPtr();

			{
				uint32_t* tmp_ptr = (uint32_t*)cur_prt;
				tmp_ptr[0] = this->m_uiUn0;
				tmp_ptr[1] = this->m_uiUn1;
				tmp_ptr[2] = this->m_uiUn2;
				tmp_ptr[3] = this->m_uiUn3;
				tmp_ptr[4] = this->m_uiStrType0Len;
				tmp_ptr[5] = this->m_uiStrType1Len;
				tmp_ptr[6] = this->m_uiStrType2Len;
			}
			cur_prt += 4 * 7;

			if (this->m_uiStrType0Len)
			{
				SPT_Char_Entry* char_entry_arry = (SPT_Char_Entry*)cur_prt;
				for (size_t ite_entry = 0; ite_entry < this->m_uiStrType0Len; ite_entry++)
				{
					char_entry_arry[ite_entry] = m_vcCharList[ite_entry];
				}

				cur_prt += m_vcCharList.size() * sizeof(SPT_Char_Entry);
			}
			if (this->m_uiStrType1Len)
			{
				memcpy(cur_prt, this->m_msStrType1.data(), this->m_uiStrType1Len);
				cur_prt += this->m_uiStrType1Len + 1;
			}
			if (this->m_uiStrType2Len)
			{
				memcpy(cur_prt, this->m_msStrType2.data(), this->m_uiStrType2Len);
				cur_prt += this->m_uiStrType2Len + 1;
			}

			return mem_data;
		}

		Rut::RxJson::Value ToJson() const
		{
			Rut::RxJson::Value json;
			json.Append(NumToHexWStr(m_uiUn0));
			json.Append(NumToHexWStr(m_uiUn1));
			json.Append(NumToHexWStr(m_uiUn2));
			json.Append(NumToHexWStr(m_uiUn3));
			json.Append(NumToHexWStr(m_uiStrType0Len));
			json.Append(NumToHexWStr(m_uiStrType1Len));
			json.Append(NumToHexWStr(m_uiStrType2Len));
			json.Append(this->ReadText(932));
			json.Append(Rut::RxStr::ToWCS(this->m_msStrType1, 932));
			json.Append(Rut::RxStr::ToWCS(this->m_msStrType2, 932));
			return json;
		}

	public:
		size_t GetSize() const
		{
			return this->m_nMemSize;
		}
	};

	class SPT_Code_Parameter_Type1
	{
	private:
		uint32_t m_uiVal_0 = 0;
		uint32_t m_uiVal_1 = 0;
		uint32_t m_uiVal_2 = 0;
		uint32_t m_uiVal_3 = 0;
		uint32_t m_uiStrLen = 0;
		uint32_t m_uiVal_5 = 0;
		uint8_t m_uiVal_6 = 0;
		uint32_t m_uiVal_7 = 0;
		std::string m_msStr;

	private:
		size_t m_nMemSize = 0;

		void CountSize()
		{
			size_t size = 0;

			size += sizeof(m_uiVal_0) + sizeof(m_uiVal_1) + sizeof(m_uiVal_2) + sizeof(m_uiVal_3) + sizeof(m_uiStrLen) + sizeof(m_uiVal_5) + sizeof(m_uiVal_6) + sizeof(m_uiVal_7);

			if (this->m_uiStrLen)
			{
				size += this->m_uiStrLen + 1;
			}

			this->m_nMemSize = size;
		}

	public:
		SPT_Code_Parameter_Type1()
		{

		}

		void Parse(uint8_t* pData)
		{
			uint8_t* cur_ptr = pData;

			{
				uint32_t* tmp_ptr = (uint32_t*)cur_ptr;
				this->m_uiVal_0 = tmp_ptr[0];
				this->m_uiVal_1 = tmp_ptr[1];
				this->m_uiVal_2 = tmp_ptr[2];
				this->m_uiVal_3 = tmp_ptr[3];
				this->m_uiStrLen = tmp_ptr[4];
				this->m_uiVal_5 = tmp_ptr[5];
			}
			cur_ptr += 6 * 4;

			this->m_uiVal_6 = *(cur_ptr);
			cur_ptr += 1;

			this->m_uiVal_7 = *(uint32_t*)(cur_ptr);
			cur_ptr += 4;

			if (this->m_uiStrLen)
			{
				char* str_ptr = (char*)(cur_ptr);
				this->m_msStr = { str_ptr, this->m_uiStrLen };
			}

			this->CountSize();
		}

		Rut::RxMem::Auto Dump() const
		{
			Rut::RxMem::Auto mem_data;
			mem_data.SetSize(this->GetSize());

			uint8_t* cur_ptr = mem_data.GetPtr();

			{
				uint32_t* tmp_ptr = (uint32_t*)cur_ptr;
				tmp_ptr[0] = this->m_uiVal_0;
				tmp_ptr[1] = this->m_uiVal_1;
				tmp_ptr[2] = this->m_uiVal_2;
				tmp_ptr[3] = this->m_uiVal_3;
				tmp_ptr[4] = this->m_uiStrLen;
				tmp_ptr[5] = this->m_uiVal_5;
			}
			cur_ptr += 6 * 4;

			*(cur_ptr) = this->m_uiVal_6;
			cur_ptr += 1;

			*(uint32_t*)(cur_ptr) = this->m_uiVal_7;
			cur_ptr += 4;

			if (this->m_uiStrLen)
			{
				char* str_ptr = (char*)(cur_ptr);
				memcpy(str_ptr, this->m_msStr.data(), this->m_uiStrLen + 1);
			}

			return mem_data;
		}

		Rut::RxJson::Value ToJson() const
		{
			Rut::RxJson::Value json;
			json.Append(NumToHexWStr(this->m_uiVal_0));
			json.Append(NumToHexWStr(this->m_uiVal_1));
			json.Append(NumToHexWStr(this->m_uiVal_2));
			json.Append(NumToHexWStr(this->m_uiVal_3));
			json.Append(NumToHexWStr(this->m_uiStrLen));
			json.Append(NumToHexWStr(this->m_uiVal_5));
			json.Append(NumToHexWStr(this->m_uiVal_6));
			json.Append(NumToHexWStr(this->m_uiVal_7));
			json.Append(Rut::RxStr::ToWCS(this->m_msStr, 932));
			return json;
		}

	public:
		size_t GetSize() const
		{
			return this->m_nMemSize;
		}
	};

	class SPT_Code_Parameter_Type2
	{
	private:
		uint32_t m_uiParameterType1Count = 0;
		std::vector<SPT_Code_Parameter_Type1> m_vcParameterType1;

	private:
		size_t m_nMemSize = 0;

		void CountSize()
		{
			size_t size = 0;

			size += sizeof(this->m_uiParameterType1Count);

			for (const auto& type1 : this->m_vcParameterType1)
			{
				size += type1.GetSize();
			}

			this->m_nMemSize = size;
		}

	public:
		SPT_Code_Parameter_Type2()
		{

		}

		void Parse(uint8_t* pData)
		{
			this->m_uiParameterType1Count = *(uint32_t*)pData;
			uint8_t* cur_ptr = pData + 4;

			for (size_t ite_type1 = 0; ite_type1 < this->m_uiParameterType1Count; ite_type1++)
			{
				SPT_Code_Parameter_Type1 type1;
				type1.Parse(cur_ptr);
				cur_ptr += type1.GetSize();
				m_vcParameterType1.emplace_back(std::move(type1));
			}

			this->CountSize();
		}

		Rut::RxMem::Auto Dump() const
		{
			Rut::RxMem::Auto mem_data;
			mem_data.SetSize(this->GetSize());

			uint8_t* data_ptr = mem_data.GetPtr();

			*(uint32_t*)data_ptr = this->m_uiParameterType1Count;
			uint8_t* cur_ptr = data_ptr + 4;

			for (const auto& type1 : this->m_vcParameterType1)
			{
				Rut::RxMem::Auto mem = type1.Dump();
				memcpy(cur_ptr, mem.GetPtr(), mem.GetSize());
				cur_ptr += mem.GetSize();
			}

			return mem_data;
		}

		Rut::RxJson::Value ToJson() const
		{
			Rut::RxJson::Value json;
			json.Append(NumToHexWStr(this->m_uiParameterType1Count));
			for (const auto& type1 : this->m_vcParameterType1)
			{
				json.Append(type1.ToJson());
			}
			return json;
		}

	public:
		size_t GetSize() const
		{
			return this->m_nMemSize;
		}

	};

	class SPT_Code_Parameter_Type3
	{
	private:
		uint32_t m_uiVal_0 = 0;
		uint32_t m_uiVal_1 = 0;
		uint32_t m_uiVal_2 = 0;

	private:
		size_t m_nMemSize = 0;

		void CountSize()
		{
			this->m_nMemSize = sizeof(m_uiVal_0) + sizeof(m_uiVal_1) + sizeof(m_uiVal_2);
		}

	public:
		SPT_Code_Parameter_Type3()
		{

		}

		void Parse(uint8_t* pData)
		{
			uint32_t* data_ptr = (uint32_t*)pData;
			this->m_uiVal_0 = data_ptr[0];
			this->m_uiVal_1 = data_ptr[1];
			this->m_uiVal_2 = data_ptr[2];

			this->CountSize();
		}

		Rut::RxMem::Auto Dump() const
		{
			Rut::RxMem::Auto mem_data;
			mem_data.SetSize(this->GetSize());

			uint32_t* data_ptr = (uint32_t*)mem_data.GetPtr();
			data_ptr[0] = this->m_uiVal_0;
			data_ptr[1] = this->m_uiVal_1;
			data_ptr[2] = this->m_uiVal_2;

			return mem_data;
		}

		Rut::RxJson::Value ToJson() const
		{
			Rut::RxJson::Value json;
			json.Append(NumToHexWStr(this->m_uiVal_0));
			json.Append(NumToHexWStr(this->m_uiVal_1));
			json.Append(NumToHexWStr(this->m_uiVal_2));
			return json;
		}

	public:
		size_t GetSize() const
		{
			return this->m_nMemSize;
		}
	};

	class SPT_Code
	{
	private:
		uint32_t m_uiCommand = 0;
		uint32_t m_uiVal_1 = 0;
		uint32_t m_uiVal_2 = 0;
		uint32_t m_uiVal_3 = 0;
		uint32_t m_uiVal_4 = 0;
		uint32_t m_uiSequnece = 0;
		uint32_t m_uiParameterType1Count = 0;
		uint32_t m_uiParameterType2Count = 0;
		uint32_t m_uiParameterType3Count = 0;

		SPT_Code_Parameter_Type0 m_ParameterType0;
		std::vector<SPT_Code_Parameter_Type1> m_vcParameterType1;
		std::vector<SPT_Code_Parameter_Type2> m_vcParameterType2;
		std::vector<SPT_Code_Parameter_Type3> m_vcParameterType3;

	private:
		size_t m_nMemSize = 0;

		void CountSize()
		{
			size_t code_size = 0;

			code_size += sizeof(this->m_uiCommand);
			code_size += sizeof(this->m_uiVal_1);
			code_size += sizeof(this->m_uiVal_2);
			code_size += sizeof(this->m_uiVal_3);
			code_size += sizeof(this->m_uiVal_4);
			code_size += sizeof(this->m_uiSequnece);
			code_size += sizeof(this->m_uiParameterType1Count);
			code_size += sizeof(this->m_uiParameterType2Count);
			code_size += sizeof(this->m_uiParameterType3Count);

			if (this->m_uiCommand == 1)
			{
				code_size += m_ParameterType0.GetSize();
			}

			for (auto& type1 : this->m_vcParameterType1)
			{
				code_size += type1.GetSize();
			}

			for (auto& type2 : this->m_vcParameterType2)
			{
				code_size += type2.GetSize();
			}

			for (auto& type3 : this->m_vcParameterType3)
			{
				code_size += type3.GetSize();
			}

			this->m_nMemSize = code_size;
		}

	public:
		SPT_Code()
		{

		}

		void Parse(uint8_t* pData)
		{
			uint32_t* data_ptr = (uint32_t*)pData;

			this->m_uiCommand = data_ptr[0];
			this->m_uiVal_1 = data_ptr[1];
			this->m_uiVal_2 = data_ptr[2];
			this->m_uiVal_3 = data_ptr[3];
			this->m_uiVal_4 = data_ptr[4];
			this->m_uiSequnece = data_ptr[5];
			this->m_uiParameterType1Count = data_ptr[6];
			this->m_uiParameterType2Count = data_ptr[7];
			this->m_uiParameterType3Count = data_ptr[8];

			uint8_t* append_data_ptr = pData + 9 * 4;

			if (m_uiCommand == 1) // Proc Text Struct
			{
				m_ParameterType0.Parse(append_data_ptr);
				append_data_ptr += m_ParameterType0.GetSize();
			}

			for (size_t ite_type1 = 0; ite_type1 < this->m_uiParameterType1Count; ite_type1++)
			{
				SPT_Code_Parameter_Type1 type1;
				type1.Parse(append_data_ptr);
				append_data_ptr += type1.GetSize();
				m_vcParameterType1.push_back(type1);
			}

			for (size_t ite_type2 = 0; ite_type2 < this->m_uiParameterType2Count; ite_type2++)
			{
				SPT_Code_Parameter_Type2 type2;
				type2.Parse(append_data_ptr);
				append_data_ptr += type2.GetSize();
				m_vcParameterType2.push_back(type2);
			}

			for (size_t ite_type3 = 0; ite_type3 < this->m_uiParameterType3Count; ite_type3++)
			{
				SPT_Code_Parameter_Type3 type3;
				type3.Parse(append_data_ptr);
				append_data_ptr += type3.GetSize();
				m_vcParameterType3.push_back(type3);
			}

			this->CountSize();
		}

		Rut::RxMem::Auto Dump() const
		{
			Rut::RxMem::Auto mem_data;
			mem_data.SetSize(this->GetSize());

			uint8_t* cur_ptr = mem_data.GetPtr();

			{
				uint32_t* tmp_ptr = (uint32_t*)cur_ptr;
				tmp_ptr[0] = this->m_uiCommand;
				tmp_ptr[1] = this->m_uiVal_1;
				tmp_ptr[2] = this->m_uiVal_2;
				tmp_ptr[3] = this->m_uiVal_3;
				tmp_ptr[4] = this->m_uiVal_4;
				tmp_ptr[5] = this->m_uiSequnece;
				tmp_ptr[6] = this->m_uiParameterType1Count;
				tmp_ptr[7] = this->m_uiParameterType2Count;
				tmp_ptr[8] = this->m_uiParameterType3Count;
			}
			cur_ptr += 9 * 4;


			if (m_uiCommand == 1) // Proc Text Struct
			{
				Rut::RxMem::Auto type0_mem = m_ParameterType0.Dump();
				memcpy(cur_ptr, type0_mem.GetPtr(), type0_mem.GetSize());
				cur_ptr += type0_mem.GetSize();
			}

			for (auto& type1 : this->m_vcParameterType1)
			{
				Rut::RxMem::Auto type1_mem = type1.Dump();
				memcpy(cur_ptr, type1_mem.GetPtr(), type1_mem.GetSize());
				cur_ptr += type1_mem.GetSize();
			}

			for (auto& type2 : this->m_vcParameterType2)
			{
				Rut::RxMem::Auto type2_mem = type2.Dump();
				memcpy(cur_ptr, type2_mem.GetPtr(), type2_mem.GetSize());
				cur_ptr += type2_mem.GetSize();
			}

			for (auto& type3 : this->m_vcParameterType3)
			{
				Rut::RxMem::Auto type3_mem = type3.Dump();
				memcpy(cur_ptr, type3_mem.GetPtr(), type3_mem.GetSize());
				cur_ptr += type3_mem.GetSize();
			}

			return mem_data;
		}

		Rut::RxJson::Value ToJson() const
		{
			Rut::RxJson::Value json;
			{
				json.Append(NumToHexWStr(this->m_uiCommand));
				json.Append(NumToHexWStr(this->m_uiVal_1));
				json.Append(NumToHexWStr(this->m_uiVal_2));
				json.Append(NumToHexWStr(this->m_uiVal_3));
				json.Append(NumToHexWStr(this->m_uiVal_4));
				json.Append(NumToHexWStr(this->m_uiSequnece));
				json.Append(NumToHexWStr(this->m_uiParameterType1Count));
				json.Append(NumToHexWStr(this->m_uiParameterType2Count));
				json.Append(NumToHexWStr(this->m_uiParameterType3Count));

				Rut::RxJson::Value json_type0;
				if (m_uiCommand == 1) // Proc Text Struct
				{
					json_type0 = std::move(m_ParameterType0.ToJson());
				}
				json.Append(std::move(json_type0));

				Rut::RxJson::Value json_type1;
				for (auto& type1 : this->m_vcParameterType1)
				{
					json_type1.Append(type1.ToJson());
				}
				json.Append(std::move(json_type1));

				Rut::RxJson::Value json_type2;
				for (auto& type2 : this->m_vcParameterType2)
				{
					json_type2.Append(type2.ToJson());
				}
				json.Append(std::move(json_type2));

				Rut::RxJson::Value json_type3;
				for (auto& type3 : this->m_vcParameterType3)
				{
					json_type3.Append(type3.ToJson());
				}
				json.Append(std::move(json_type3));
			}
			return json;
		}

	public:
		size_t GetSize() const
		{
			return this->m_nMemSize;
		}

	};

	class SPT_Append_Script_Info
	{
	private:
		uint32_t m_uiStrLen0 = 0;
		std::string m_msStr0;
		uint32_t m_uiStrLen1 = 0;
		std::string m_msStr1;
		uint32_t m_uiUn0 = 0;
		uint32_t m_uiUn1 = 0;
		uint32_t m_uiUn2 = 0;
		char m_aAppend[0x80] = {}; // Maybe there's no data in here.

	private:
		size_t m_nMemSize = 0;

		void CountSize()
		{
			this->m_nMemSize = sizeof(this->m_uiStrLen0) + m_msStr0.size() + sizeof(this->m_uiStrLen1) + m_msStr1.size() + sizeof(m_uiUn0) + sizeof(m_uiUn1) + sizeof(m_uiUn2) + sizeof(this->m_aAppend);
		}

	public:
		SPT_Append_Script_Info()
		{

		}

		void Parse(uint8_t* pData)
		{
			uint8_t* cur_ptr = pData;

			this->m_uiStrLen0 = *(uint32_t*)cur_ptr;
			cur_ptr += 4;

			if (this->m_uiStrLen0)
			{
				char* str0_ptr = (char*)(cur_ptr);
				this->m_msStr0 = { str0_ptr , this->m_uiStrLen0 };
			}
			cur_ptr += this->m_uiStrLen0;

			this->m_uiStrLen1 = *(uint32_t*)cur_ptr;
			cur_ptr += 4;

			if (this->m_uiStrLen1)
			{
				char* str1_ptr = (char*)(cur_ptr);
				this->m_msStr1 = { str1_ptr , this->m_uiStrLen1 };
			}
			cur_ptr += this->m_uiStrLen1;

			this->m_uiUn0 = *(uint32_t*)cur_ptr;
			cur_ptr += 4;

			this->m_uiUn1 = *(uint32_t*)cur_ptr;
			cur_ptr += 4;

			this->m_uiUn2 = *(uint32_t*)cur_ptr;
			cur_ptr += 4;

			memcpy(this->m_aAppend, cur_ptr, sizeof(this->m_aAppend));

			this->CountSize();
		}

		Rut::RxMem::Auto Dump() const
		{
			Rut::RxMem::Auto mem_data;
			mem_data.SetSize(this->GetSize());
			uint8_t* cur_ptr = mem_data.GetPtr();
			{
				*(uint32_t*)cur_ptr = this->m_uiStrLen0;
				cur_ptr += 4;

				if (this->m_uiStrLen0)
				{
					memcpy(cur_ptr, this->m_msStr0.data(), this->m_uiStrLen0);
				}
				cur_ptr += this->m_uiStrLen0;

				*(uint32_t*)cur_ptr = this->m_uiStrLen1;
				cur_ptr += 4;

				if (this->m_uiStrLen1)
				{
					memcpy(cur_ptr, this->m_msStr1.data(), this->m_uiStrLen1);
				}
				cur_ptr += this->m_uiStrLen1;

				*(uint32_t*)cur_ptr = this->m_uiUn0;
				cur_ptr += 4;

				*(uint32_t*)cur_ptr = this->m_uiUn1;
				cur_ptr += 4;

				*(uint32_t*)cur_ptr = this->m_uiUn2;
				cur_ptr += 4;

				memcpy(cur_ptr, this->m_aAppend, sizeof(this->m_aAppend));
			}
			return mem_data;
		}

		Rut::RxJson::Value ToJson() const
		{
			Rut::RxJson::Value json;
			{
				json.Append(NumToHexWStr(this->m_uiStrLen0));
				json.Append(Rut::RxStr::ToWCS(this->m_msStr0, 932));
				json.Append(NumToHexWStr(this->m_uiStrLen1));
				json.Append(Rut::RxStr::ToWCS(this->m_msStr1, 932));
				json.Append(NumToHexWStr(this->m_uiUn0));
				json.Append(NumToHexWStr(this->m_uiUn1));
				json.Append(NumToHexWStr(this->m_uiUn2));

				Rut::RxJson::Value json_append;
				uint32_t append_size = (sizeof(this->m_aAppend) / 4);
				uint32_t* append_ptr = (uint32_t*)this->m_aAppend;
				for (size_t ite_append = 0; ite_append < append_size; ite_append++)
				{
					json_append.Append(NumToHexWStr(append_ptr[ite_append]));
				}
				json.Append(json_append);
			}
			return json;
		}

	public:
		size_t GetSize() const
		{
			return this->m_nMemSize;
		}
	};

	class SPT_Append_Script
	{
	private:
		uint32_t m_uiScriptCount = 0;
		std::vector<SPT_Append_Script_Info> m_vcInfo;

	private:
		size_t m_nMemSize = 0;

		void CountSize()
		{
			size_t size = 0;

			size += sizeof(this->m_uiScriptCount);

			for (auto& info : this->m_vcInfo)
			{
				size += info.GetSize();
			}

			this->m_nMemSize = size;
		}

	public:
		SPT_Append_Script()
		{

		}

		void Parse(uint8_t* pData)
		{
			uint8_t* cur_prt = pData;

			this->m_uiScriptCount = *(uint32_t*)pData;
			cur_prt += 4;

			for (size_t ite_info = 0; ite_info < this->m_uiScriptCount; ite_info++)
			{
				SPT_Append_Script_Info info;
				info.Parse(cur_prt);
				cur_prt += info.GetSize();
				m_vcInfo.emplace_back(std::move(info));
			}

			this->CountSize();
		}

		Rut::RxMem::Auto Dump() const
		{
			Rut::RxMem::Auto mem_data;
			mem_data.SetSize(this->GetSize());
			uint8_t* cur_ptr = mem_data.GetPtr();
			{
				*(uint32_t*)cur_ptr = this->m_uiScriptCount;
				cur_ptr += 4;

				for (const auto& info : this->m_vcInfo)
				{
					Rut::RxMem::Auto data = info.Dump();
					memcpy(cur_ptr, data.GetPtr(), data.GetSize());
					cur_ptr += data.GetSize();
				}
			}
			return mem_data;
		}

		Rut::RxJson::Value ToJson() const
		{
			Rut::RxJson::Value json;
			{
				json.Append(NumToHexWStr(this->m_uiScriptCount));

				Rut::RxJson::Value json_info;
				for (const auto& info : this->m_vcInfo)
				{
					json_info.Append(info.ToJson());
				}
				json.Append(json_info);
			}
			return json;
		}

	public:
		size_t GetSize() const
		{
			return this->m_nMemSize;
		}
	};

	class SPT_Encryptor_Info
	{
	private:
		uint8_t m_ucDecStartIndex = 0;
		uint8_t m_ucDecModeType = 0;
		uint8_t m_ucUn0 = 0;
		uint8_t m_ucUn1 = 0;

	private:
		size_t m_nMemSize = 0;

		void CountSize()
		{
			this->m_nMemSize = sizeof(m_ucDecStartIndex) + sizeof(m_ucDecModeType) + sizeof(m_ucUn0) + sizeof(m_ucUn1);
		}

	public:
		SPT_Encryptor_Info()
		{

		}

		void Parse(uint8_t* pData)
		{
			this->m_ucDecStartIndex = pData[0];
			this->m_ucDecModeType = pData[1];
			this->m_ucUn0 = pData[2];
			this->m_ucUn1 = pData[3];

			this->CountSize();
		}

		Rut::RxMem::Auto Dump() const
		{
			Rut::RxMem::Auto mem_data;
			mem_data.SetSize(this->GetSize());
			uint8_t* cur_ptr = mem_data.GetPtr();
			{
				cur_ptr[0] = this->m_ucDecStartIndex;
				cur_ptr[1] = this->m_ucDecModeType;
				cur_ptr[2] = this->m_ucUn0;
				cur_ptr[3] = this->m_ucUn1;
			}
			return mem_data;
		}

		Rut::RxJson::Value ToJson() const
		{
			Rut::RxJson::Value json;
			{
				json.Append(NumToHexWStr(this->m_ucDecStartIndex));
				json.Append(NumToHexWStr(this->m_ucDecModeType));
				json.Append(NumToHexWStr(this->m_ucUn0));
				json.Append(NumToHexWStr(this->m_ucUn1));
			}
			return json;
		}

	public:
		size_t GetSize() const
		{
			return this->m_nMemSize;
		}
	};

	class SPT_Codes_Info
	{
	private:
		uint32_t m_uiCodeCount = 0;
		uint32_t m_uiUn0 = 0;
		uint32_t m_uiUn1 = 0;
		uint32_t m_uiUn2 = 0;

	private:
		size_t m_nMemSize = 0;

		void CountSize()
		{
			this->m_nMemSize = sizeof(m_uiCodeCount) + sizeof(m_uiUn0) + sizeof(m_uiUn1) + sizeof(m_uiUn2);
		}

	public:
		SPT_Codes_Info()
		{

		}

		void Parse(uint8_t* pInfo)
		{
			uint32_t* info_chunk_ptr = (uint32_t*)pInfo;

			this->m_uiCodeCount = info_chunk_ptr[0];
			this->m_uiUn0 = info_chunk_ptr[1];
			this->m_uiUn1 = info_chunk_ptr[2];
			this->m_uiUn2 = info_chunk_ptr[3];

			this->CountSize();
		}

		Rut::RxMem::Auto Dump() const
		{
			Rut::RxMem::Auto mem_data;
			mem_data.SetSize(this->GetSize());
			uint8_t* cur_ptr = mem_data.GetPtr();
			{
				uint32_t* tmp_ptr = (uint32_t*)cur_ptr;
				tmp_ptr[0] = this->m_uiCodeCount;
				tmp_ptr[1] = this->m_uiUn0;
				tmp_ptr[2] = this->m_uiUn1;
				tmp_ptr[3] = this->m_uiUn2;
			}
			return mem_data;
		}

		Rut::RxJson::Value ToJson() const
		{
			Rut::RxJson::Value json;
			{
				json.Append(NumToHexWStr(this->m_uiCodeCount));
				json.Append(NumToHexWStr(this->m_uiUn0));
				json.Append(NumToHexWStr(this->m_uiUn1));
				json.Append(NumToHexWStr(this->m_uiUn2));
			}
			return json;
		}

	public:
		size_t GetSize() const
		{
			return this->m_nMemSize;
		}

		size_t GetCodeCount() const
		{
			return this->m_uiCodeCount;
		}
	};

	class SPT_HDR
	{
	private:
		SPT_Encryptor_Info m_EncryptorInfo;
		uint32_t m_uiUnCount = 0;
		uint32_t m_uiScriptNameLen = 0;
		std::string m_msScriptName;
		SPT_Codes_Info m_CodesInfo;
		SPT_Append_Script m_aAppendScript[0xF];
		uint32_t m_uiUnsize = 0;

	private:
		size_t m_nMemSize = 0;

		void CountSize()
		{
			size_t size = 0;
			{
				size += this->m_EncryptorInfo.GetSize() + sizeof(this->m_uiUnCount) + sizeof(this->m_uiScriptNameLen) + this->m_msScriptName.size() + this->m_CodesInfo.GetSize();
				for (auto& append_script : this->m_aAppendScript)
				{
					size += append_script.GetSize();
				}
				size += sizeof(this->m_uiUnsize);
			}
			this->m_nMemSize = size;
		}

	public:
		SPT_HDR()
		{

		}

		void Parse(uint8_t* pData)
		{
			uint8_t* cur_ptr = pData;

			this->m_EncryptorInfo.Parse(pData);
			cur_ptr += this->m_EncryptorInfo.GetSize();

			this->m_uiUnCount = *((uint32_t*)(cur_ptr));
			cur_ptr += 4;

			this->m_uiScriptNameLen = *(uint32_t*)(cur_ptr);
			cur_ptr += 4;

			char* script_name_ptr = (char*)(cur_ptr);
			this->m_msScriptName = { script_name_ptr, this->m_uiScriptNameLen };
			cur_ptr += this->m_uiScriptNameLen;

			this->m_CodesInfo.Parse(cur_ptr);
			cur_ptr += m_CodesInfo.GetSize();

			for (auto& append_script : this->m_aAppendScript)
			{
				append_script.Parse(cur_ptr);
				cur_ptr += append_script.GetSize();
			}

			this->m_uiUnsize = *(uint32_t*)cur_ptr;

			this->CountSize();
		}

		Rut::RxMem::Auto Dump() const
		{
			Rut::RxMem::Auto mem_data;
			mem_data.SetSize(this->GetSize());
			uint8_t* cur_ptr = mem_data.GetPtr();
			{
				Rut::RxMem::Auto enc_info_mem = this->m_EncryptorInfo.Dump();
				memcpy(cur_ptr, enc_info_mem.GetPtr(), enc_info_mem.GetSize());
				cur_ptr += enc_info_mem.GetSize();

				*((uint32_t*)(cur_ptr)) = this->m_uiUnCount;
				cur_ptr += 4;

				*(uint32_t*)(cur_ptr) = this->m_uiScriptNameLen;
				cur_ptr += 4;

				memcpy(cur_ptr, this->m_msScriptName.data(), this->m_uiScriptNameLen);
				cur_ptr += this->m_uiScriptNameLen;

				Rut::RxMem::Auto codes_info_mem = this->m_CodesInfo.Dump();
				memcpy(cur_ptr, codes_info_mem.GetPtr(), codes_info_mem.GetSize());
				cur_ptr += codes_info_mem.GetSize();

				for (auto& append_script : this->m_aAppendScript)
				{
					Rut::RxMem::Auto append_script_mem = append_script.Dump();
					memcpy(cur_ptr, append_script_mem.GetPtr(), append_script_mem.GetSize());
					cur_ptr += append_script_mem.GetSize();
				}

				*(uint32_t*)cur_ptr = this->m_uiUnsize;
			}
			return mem_data;
		}

		Rut::RxJson::Value ToJson() const
		{
			Rut::RxJson::Value json;
			{
				json.Append(this->m_EncryptorInfo.ToJson());
				json.Append(NumToHexWStr(this->m_uiUnCount));
				json.Append(NumToHexWStr(this->m_uiScriptNameLen));
				json.Append(Rut::RxStr::ToWCS(this->m_msScriptName, 932));
				json.Append(m_CodesInfo.ToJson());
				
				Rut::RxJson::Value json_append_script;
				for (const auto& append_script : this->m_aAppendScript)
				{
					json_append_script.Append(append_script.ToJson());
				}
				json.Append(json_append_script);

				json.Append(NumToHexWStr(this->m_uiUnsize));
			}
			return json;
		}

	public:
		size_t GetSize() const
		{
			return this->m_nMemSize;
		}

		size_t GetCodeCount() const
		{
			return this->m_CodesInfo.GetCodeCount();
		}


	};

	class Parser
	{
	private:
		SPT_HDR m_HDR;
		std::vector<SPT_Code> m_vcCode;

	private:
		size_t m_nMemSize = 0;

		void CountSize()
		{
			size_t size = 0;
			{
				size += this->m_HDR.GetSize();
				for (const auto& code : this->m_vcCode)
				{
					size += code.GetSize();
				}
			}
			this->m_nMemSize = size;
		}

	public:
		Parser()
		{

		}

		void Parse(uint8_t* pData)
		{
			uint8_t* cur_ptr = pData;

			this->m_HDR.Parse(cur_ptr);
			cur_ptr += this->m_HDR.GetSize();

			size_t code_count = this->m_HDR.GetCodeCount();
			for (size_t ite_chunk = 0; ite_chunk < code_count; ite_chunk++)
			{
				SPT_Code code;
				code.Parse(cur_ptr);
				cur_ptr += code.GetSize();
				m_vcCode.emplace_back(std::move(code));
			}

			this->CountSize();
		}

		Rut::RxMem::Auto Dump() const
		{
			Rut::RxMem::Auto mem_data;
			mem_data.SetSize(this->GetSize());
			uint8_t* cur_ptr = mem_data.GetPtr();
			{
				Rut::RxMem::Auto hdr_mem = this->m_HDR.Dump();
				memcpy(cur_ptr, hdr_mem.GetPtr(), hdr_mem.GetSize());
				cur_ptr += hdr_mem.GetSize();

				for (const auto& code : this->m_vcCode)
				{
					Rut::RxMem::Auto code_mem = code.Dump();
					memcpy(cur_ptr, code_mem.GetPtr(), code_mem.GetSize());
					cur_ptr += code_mem.GetSize();
				}
			}
			return mem_data;
		}

	public:
		size_t GetSize() const
		{
			return this->m_nMemSize;
		}
	};
}
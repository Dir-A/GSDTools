struct GSD_Pack_Info
{
	uint32_t uiFOA;
	uint32_t uiSize;
}


struct GSD_Pack_Entry
{
	GSD_Pack_Info Info;
	char aFileName[0x38];
}

struct GSD_Pack_HDR
{
	uint32_t uiEntryCount;
	GSD_Pack_Entry aEntry[uiEntryCount];
}

union GSD_STD_RawStr
{
  char *cpStr;
  char aStr[16];
};


struct GSD_STD_String
{

	GSD_STD_RawStr uStr;
	uint32_t uiLen;
	uint32_t uiReserve;
}

struct GSD_FS_Pack_Entry
{
	uint64_t uiFOA;
	uint64_t uiSize;
	char aFileName[0x70];
}

struct __declspec(align(8)) GSD_FS
{
  HANDLE hFile;
  uint32_t uiFilePointer_Back;
  uint32_t hFile_0;
  LARGE_INTEGER sOldFilePointer;
  uint32_t aUn0[3];
  LARGE_INTEGER sFilePointer;
  LARGE_INTEGER sFileSize;
  uint32_t hFile_1;
  uint32_t aUn1[28];
  uint32_t hFile_2;
  uint32_t aUn2[6];
  uint32_t hFile_3;
  uint32_t aUn3[6];
  uint32_t hFile_4;
  uint32_t aUn4[6];
  uint32_t uiSlot1_Handle;
  uint32_t uiSlot1_Un0;
  uint32_t uiSlot1_Handle_0;
  uint32_t aUn5[49];
  uint32_t pResSize;
  uint32_t nDecompressSize;
  uint32_t pResBuffer_Active;
  uint32_t uiEntryCount;
  uint32_t uiIsEntryMaxCount;
  uint32_t uiUnx;
GSD_FS_Pack_Entry memEntry;
  uint8_t *pResBuffer_Org;
  uint32_t uiUnSize;
  uint32_t aUn6[52];
};

strutc GSD_Spt
{
	uint8_t ucDecBegIndex;
	uint8_t ucType;
	uint8_t ucUn1;
	uint8_t ucUn2;
}

struct GSD_Script_Decoder_One
{
	uint32_t aKey0[4];
}

struct GSD_Script_Decoder
{
	GSD_Script_Decoder_One[16];
}

脚本偏移0x60进入第一个op
op -> 0x24字节

01000000 --> 判断文本消息结果位
00000000
00000000
00000000
00000000
13000000 --> 有可能只是个行号，随着文本次序递增
00000000
00000000
00000000

FFFFFFFF --> 字符开始？没懂啥玩意，好像没什么实际作用
FFFFFFFF
FFFFFFFF
FFFFFFFF

09000000 --> 字符个数
00000000
00000000

07000000
00000000
81770000 --> 字符

07000000
00000000
82E20000 --> 字符

07000000
00000000
829F0000 --> 字符

07000000
00000000
82C10000 --> 字符

07000000
00000000
82C10000 --> 字符

07000000
00000000
81490000 --> 字符

07000000
00000000
81490000 --> 字符

07000000
00000000
81780000 --> 字符

0D000000 --> 结束标志 '\n'
00000000
00000000


https://github.com/shangjiaxuan/Crass-source/blob/4aff113b98fc39fb85f64501ab47c580df779a3d/cui-1.0.4/AGSD/AGSD.cpp
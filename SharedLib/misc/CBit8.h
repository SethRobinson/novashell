#ifndef CBit8_h__
#define CBit8_h__


//simple thing to read and write 8 bools from a byte

#define D_BIT_0 1 	//00000001 in bin
#define D_BIT_1 2 	
#define D_BIT_2 4 	
#define D_BIT_3 8 	
#define D_BIT_4 16 	
#define D_BIT_5 32 	
#define D_BIT_6 64 	

#define D_BIT_7 128	

//for use with the 32 version

#define D_BIT_8 256	
#define D_BIT_9 512	
#define D_BIT_10 1024	
#define D_BIT_11 2048	
#define D_BIT_12 4096	
#define D_BIT_13 8192	
#define D_BIT_14 16384	
#define D_BIT_15 32768	
#define D_BIT_16 65536	
#define D_BIT_17 131072	
#define D_BIT_18 262144	
#define D_BIT_19 524288
#define D_BIT_20 1048576
#define D_BIT_21 2097152
#define D_BIT_22 4194304
#define D_BIT_23 8388608
#define D_BIT_24 16777216


class CBit8
{
private:
	unsigned char byte_this;
public:


	void clear()
	{
	  byte_this = 0;
	};

	void set_bit(unsigned char d_bit, bool b_status)
	{
	  if (b_status)
	  {
	      //set bit
		  byte_this |= d_bit;
	  } else
	  {
	    //clear bit
 		  byte_this &= ~d_bit;
	  }

	};


   bool get_bit(unsigned char d_bit)
	{
	  if (byte_this & d_bit) return true; else return false;
	};


};


class CBit32
{
private:
	unsigned int byte_this;
public:

	bool operator != (const CBit32& v) const
	{
		return !(byte_this == (v.byte_this));
	}

	void clear()
	{
		byte_this = 0;
	};

	void set_bit(unsigned int d_bit, bool b_status)
	{
		if (b_status)
		{
			//set bit
			byte_this |= d_bit;
		} else
		{
			//clear bit
			byte_this &= ~d_bit;
		}

	};


	bool get_bit(unsigned int d_bit)
	{
		if (byte_this & d_bit) return true; else return false;
	};


};

#endif // CBit8_h__

#define __ADDRESSBOOK_C_
#include "version.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>

#include "addressbook.h"
#include "char.h"
#include "handletime.h"
#include "buf.h"
#include "net.h"
#include "lssproto_serv.h"
#include "saacproto_cli.h"
#include "object.h"
#include "battle.h"
#include "configfile.h"
#include "npcutil.h"
#include "pet.h"
#include "petmail.h"
#include "log.h"

/*裔爛丟永本□斥及  贏贏今〔票匹爛聒允月  儂  及贏今反
 仇木動票卞允月仇午*/
#define ADDRESSBOOK_FIXEDMESSAGE_MAXLEN  128

/* 憤坌及蟆卞簿手中卅井勻凶及匹｝失玉伊旦皮永弁卞馨笛匹五卅井勻凶
   午五及裔爛丟永本□斥 */
#define ADDRESSBOOK_CANTADD "那裡沒有任何人。"
#define ADDRESSBOOK_CANTADD2 "無法交換名片。"

/* 簿井毛笛尹月仇午互匹五凶午五｝笛尹方丹午仄凶諦卞霜耨允月丟永本□斥*/
#define ADDRESSBOOK_ADDED "和%s交換名片 。"

/* 簿井卞湧毛創尹日木凶日 */
#define ADDRESSBOOK_BEINGADDED "和%s交換名片 。"

/* 巨件玄伉互中勻天中分勻凶午五及丟永本□斥 */
#define ADDRESSBOOK_MYTABLEFULL "名片匣已滿。"

/* 鍬澎及巨件玄伉互中勻天中分勻凶午五及丟永本□斥 */
#define ADDRESSBOOK_HISTABLEFULL "對方的名片匣已滿。"


/* 丟永本□斥毛霜耨允月及卞岳  仄凶午五 */
#define ADDRESSBOOK_SENT  "送信給%s 。"

/* 丟永本□斥毛霜耨允月及卞撩  仄凶午五 */
#define ADDRESSBOOK_UNSENT  "無法送信給%s 。"

/* 簿井毛創尹方丹午仄凶互｝湃卞創尹化中凶   */
#define ADDRESSBOOK_ALREADYADDED  "已經和%s交換過名片了。 "

/*   銅毛域  讀卞  丹橇謫   */
#define ADDRESSBOOK_GIVEADDRESS  "從%s得到名片。"

/*   銅毛域  讀卞丐僕月橇謫   */
#define ADDRESSBOOK_TAKEADDRESS1  "給%s自己的名片。"
/*   銅毛域  讀卞丐僕月橇謫   */
#define ADDRESSBOOK_TAKEADDRESS2  "因為%s想要名片，所以將名片給他了。"

#define	ADDRESSBOOK_RETURNED1	\
"從%s寄來信件，但由於沒有%s的名片又將信件退回。"

#define	ADDRESSBOOK_RETURNED2	\
"寄信件給%s，但由於%s 沒有名片，所以信件又被退回來了。"

#define	PETMAIL_RETURNED1	\
"%s不在這個世界裡，所以無法寄送信件給他。"


/* static匹銀丹迕〔  五中襖手*/
char ADDRESSBOOK_returnstring[25*128];



static int ADDRESSBOOK_findBlankEntry( int cindex );
static BOOL ADDRESSBOOK_makeEntryFromCharaindex( int charaindex,
												 ADDRESSBOOK_entry* ae);

/*------------------------------------------------------------
 * 失玉伊旦皮永弁及丟永本□斥毛霜耨允月
 * MSG皿夫玄戊伙井日勾井歹木月〔
 *
 * 支月仇午反｝connection井日cdkey匹腹綢仄化｝平乓仿  手
 * 甲永玄仄凶日｝ MSG_send允月〔公及午五卞｝憤坌及樹  互
 * 鍬澎及伉旦玄卞卅井勻凶日窒手仄卅中午中丹仇午分〔
 * 婁醒
 *  cindex  int     平乓仿及index
 *  aindex  int     失玉伊旦皮永弁及index
 *  text    char*   霜耨允月  儂  
 *  color   int     縉
 * 忒曰襖
 * 左件仿奶件及平乓仿卞丟永本□斥毛霜耨仄凶日TRUE ,
 * 左白仿奶件卞瓚  仄凶日FALSE毛井尹允
 ------------------------------------------------------------*/
BOOL ADDRESSBOOK_sendMessage( int cindex, int aindex, char* text , int color )
{
	int 	i ;
	char 	tmpmsg[256];
	char	textbuffer[2048];
	char	*mycd;
	char 	*mycharaname = CHAR_getChar(cindex,CHAR_NAME );
	struct  tm tm1;
	ADDRESSBOOK_entry *ae;
	int     playernum = CHAR_getPlayerMaxNum();

	if( !CHAR_CHECKINDEX(cindex) )return FALSE;

	ae = CHAR_getAddressbookEntry( cindex , aindex );
	if( ae == NULL )return FALSE;

	//getcdkeyFromCharaIndex(cindex, mycd,sizeof(mycd) );
	mycd = CHAR_getChar( cindex, CHAR_CDKEY);
     
	memcpy( &tm1, localtime( (time_t *)&NowTime.tv_sec), sizeof( tm1));

    snprintf( textbuffer, sizeof( textbuffer), 
    		"%2d/%02d %2d:%02d|%s|-1", 
    		tm1.tm_mon +1, tm1.tm_mday, tm1.tm_hour, tm1.tm_min,
    		text);
	
	/*   扔□田□  卞中月凜 */
	for( i = 0 ; i < playernum ; i ++){
		if( CHAR_CHECKINDEX( i) &&
			strcmp( CHAR_getChar( i, CHAR_CDKEY), ae->cdkey) == 0 &&
			strcmp( CHAR_getChar( i, CHAR_NAME), ae->charname) == 0 )
		{
			/*
			 * CDKEY 手 平乓仿  手域譙仄凶〔公及平乓仿弁正及
			 * 失玉伊旦皮永弁卞憤坌及樹  互丐月井譬屯化｝
			 * 繡箕仄凶日｝MSG允月〔
			 */
			int index_to_my_info = 
					ADDRESSBOOK_getIndexInAddressbook( i , 
														mycd, mycharaname);

			int		fd;
			if( index_to_my_info < 0 ){
				/*
				 * 鍬澎互憤坌毛坫壅仄化仄引勻化月〔
				 * 域殺  諦卞〕丟□伙互  凶午分仃騷襞允月〔
				 */
				//snprintf( tmpmsg, sizeof( tmpmsg), 
				//		  ADDRESSBOOK_RETURNED1,
				//			ae->charname
				//			);

				//CHAR_talkToCli( CONNECT_getCharaindex(i), -1, 
				//				tmpmsg , CHAR_COLORYELLOW );

				snprintf( tmpmsg, sizeof( tmpmsg), 
						  ADDRESSBOOK_RETURNED2,
							CHAR_getChar( i, CHAR_NAME),
							CHAR_getChar( i, CHAR_NAME));

				/* 霜曰潛卞手丟永本□斥 */
				CHAR_talkToCli( cindex, -1, 
								tmpmsg , CHAR_COLORYELLOW );
				return FALSE;
			}
			
			fd = getfdFromCharaIndex( i);
			if( fd != -1 ) {
				lssproto_MSG_send( fd , index_to_my_info , textbuffer , color );
				/* 夫弘午曰 */
				printl( LOG_TALK, "CD=%s\tNM=%s\tT=%s" , mycd, mycharaname, textbuffer );
			
			}

			snprintf( tmpmsg , sizeof( tmpmsg),ADDRESSBOOK_SENT,
					  ae->charname  );
			CHAR_talkToCli(cindex,-1, tmpmsg , color );

            // WON ADD 修正snprintf會導致當機的bug
			{
					char tmp[1000];
					sprintf( tmp , "ADDRESSBOOK_sendMessage:"
						 "Send MSG to: %s %s\n",
						 ae->cdkey , ae->charname );
				//	print( tmp );
			}

			CHAR_setInt( cindex, CHAR_SENDMAILCOUNT, 
						CHAR_getInt( cindex, CHAR_SENDMAILCOUNT)+1);
			return TRUE;
		}
	}
	/* 葦勾井日卅井勻凶凜反〕失市它件玄扔□田□卞霜月 */
	saacproto_Message_send( acfd, mycd, mycharaname, 
							ae->cdkey, ae->charname, textbuffer, color);
	CHAR_setInt( cindex, CHAR_SENDMAILCOUNT, 
				CHAR_getInt( cindex, CHAR_SENDMAILCOUNT)+1);


	snprintf( tmpmsg , sizeof( tmpmsg),ADDRESSBOOK_SENT,ae->charname  );
	CHAR_talkToCli( cindex,-1, tmpmsg , CHAR_COLORWHITE );

	return FALSE;
}
/*------------------------------------------------------------
 * 失玉伊旦皮永弁及丟永本□斥毛霜耨允月
 * saac 井日msg 毛熬仃午勻化弁仿奶失件玄卞禾旦玄允月〔
 *
 * 忒曰襖
 ------------------------------------------------------------*/
BOOL ADDRESSBOOK_sendMessage_FromOther( char *fromcdkey, char *fromcharaname, 
										char *tocdkey, char *tocharaname,
										char* text , int color )
{
#define		ADDRESSBOOK_SYSTEM			"system"

	int i ;
	char tmpmsg[256];
	int     playernum = CHAR_getPlayerMaxNum();
	
	/* 扔□田□  毛腹綢允月 */
	for( i = 0 ; i < playernum ; i ++){
		if( CHAR_CHECKINDEX( i) &&
			strcmp( CHAR_getChar( i, CHAR_CDKEY), tocdkey) == 0 &&
			strcmp( CHAR_getChar( i, CHAR_NAME), tocharaname) == 0)
		{
			int		index_to_my_info;
			/*
			 * CDKEY 手 平乓仿  手域譙仄凶〔公及平乓仿弁正及
			 * 失玉伊旦皮永弁卞憤坌及樹  互丐月井譬屯化｝
			 * 繡箕仄凶日｝MSG允月〔
			 */
			 
			/* 撲旦  丞丟永本□斥互窖勻化五凶 */
			if( strcmp( fromcdkey, ADDRESSBOOK_SYSTEM) == 0 &&
				strcmp( fromcharaname, ADDRESSBOOK_SYSTEM ) == 0 ) 
			{
				/* 撲旦  丞丟永本□斥毛龔仁 */
				CHAR_talkToCli( i, -1, text , color );
				break;
			}
			
			index_to_my_info = 
					ADDRESSBOOK_getIndexInAddressbook( i , 
														fromcdkey, fromcharaname);
			if( index_to_my_info < 0 ){
				/*
				 * 鍬澎互憤坌毛坫壅仄化仄引勻化月〔
				 */

				snprintf( tmpmsg, sizeof( tmpmsg), ADDRESSBOOK_RETURNED2,
							tocharaname, tocharaname);

				/* 霜曰潛卞手丟永本□斥 */
				saacproto_Message_send( acfd, ADDRESSBOOK_SYSTEM , ADDRESSBOOK_SYSTEM, 
										fromcdkey, fromcharaname, tmpmsg, CHAR_COLORYELLOW);

			}
			else {
				int fd = getfdFromCharaIndex( i);
				if( fd != -1 ) {
					lssproto_MSG_send( fd , index_to_my_info , text , color );
					/* 夫弘午曰 */
					printl( LOG_TALK, "CD=%s\tNM=%s\tT=%s" , fromcdkey,
															fromcharaname, text );
				}
			}
			break;
		}
	}
	if( i == playernum ) return FALSE;
	return TRUE;
}

int ADDRESSBOOK_getIndexInAddressbook(int cindex , char *cdkey,
											 char *charname)
{
	int i ;

	if( !CHAR_CHECKINDEX( cindex ) ) return -1;

	for( i = 0 ; i < ADDRESSBOOK_MAX ; i++){
		ADDRESSBOOK_entry *ae = CHAR_getAddressbookEntry( cindex , i );
		if( ae && ae->use && strcmp( ae->cdkey, cdkey )==0 &&
			strcmp( ae->charname , charname ) == 0 ){
			return i;
		}
	}
	return -1;
}

BOOL ADDRESSBOOK_deleteEntry( int meindex ,int index )
{
	ADDRESSBOOK_entry ent;
	BOOL ret;
	if( !CHAR_CHECKINDEX( meindex ) ) return FALSE;

	memset( &ent ,0, sizeof( ADDRESSBOOK_entry ));
	ret = CHAR_setAddressbookEntry( meindex , index , &ent );
	if( ret == TRUE ){
		ADDRESSBOOK_sendAddressbookTable( meindex );
		return TRUE;
	}
	return FALSE;
}

BOOL ADDRESSBOOK_addEntry( int meindex )
{
	int objbuf[20];
	int found_count;
	int front_x , front_y;
	int i;
	int	cnt = 0;
	int	fd;
    char *mycd , *tocd;
	BOOL found = FALSE;

	if( !CHAR_CHECKINDEX( meindex ) )return FALSE;
	
	fd = getfdFromCharaIndex( meindex);
	if( fd == -1 ) return FALSE;

	if( ADDRESSBOOK_findBlankEntry( meindex ) < 0) {
		CHAR_talkToCli( meindex , -1,ADDRESSBOOK_MYTABLEFULL,CHAR_COLORWHITE );
		return FALSE;
	}

	for( i = 0; i < CONNECT_WINDOWBUFSIZE; i ++ ) {
        CONNECT_setTradecardcharaindex( fd,i,-1);
    }

	CHAR_getCoordinationDir( CHAR_getInt( meindex, CHAR_DIR ) ,
							 CHAR_getInt( meindex , CHAR_X ),
							 CHAR_getInt( meindex , CHAR_Y ) ,
							 1 , &front_x , &front_y );

	found_count = CHAR_getSameCoordinateObjects( objbuf,
											arraysizeof( objbuf),
											CHAR_getInt(meindex,CHAR_FLOOR),
											front_x,front_y );
	if( found_count == 0 ){
		CHAR_talkToCli( meindex, -1, ADDRESSBOOK_CANTADD, CHAR_COLORWHITE);
		return FALSE;
	}
	for( i = 0 ; i < found_count; i++ ){
		int objindex = objbuf[i];
		int	index = OBJECT_getIndex( objindex);
		if( OBJECT_getType(objindex) != OBJTYPE_CHARA ) {
			continue;
		}
		if( CHAR_getInt( index,CHAR_WHICHTYPE ) != CHAR_TYPEPLAYER ){
			continue;
		}
		if( index == meindex ) {
			continue;
		}
		found = TRUE;
  		if( CHAR_getWorkInt( index, CHAR_WORKBATTLEMODE) != BATTLE_CHARMODE_NONE) {
  			continue;
  		}
  		if(!CHAR_getFlg( index, CHAR_ISTRADECARD)) {
  			continue;
  		}
		if( ADDRESSBOOK_findBlankEntry( index ) < 0 ) {
			continue;
		}
        tocd = CHAR_getChar( index, CHAR_CDKEY);
        mycd = CHAR_getChar( meindex, CHAR_CDKEY);
		if( ADDRESSBOOK_getIndexInAddressbook( meindex, tocd,
									CHAR_getChar( index, CHAR_NAME)) >= 0 &&
			ADDRESSBOOK_getIndexInAddressbook( index, mycd,
										CHAR_getChar(meindex, CHAR_NAME) )  >= 0  )	{
			continue;
		}

        CONNECT_setTradecardcharaindex( fd,cnt,index);
		cnt ++;
		if( cnt == CONNECT_WINDOWBUFSIZE ) break;
	}

	if( cnt == 0 ) {
		if( found ) {
			CHAR_talkToCli( meindex, -1, ADDRESSBOOK_CANTADD2, CHAR_COLORWHITE);
		}else {
			CHAR_talkToCli( meindex, -1, ADDRESSBOOK_CANTADD, CHAR_COLORWHITE);
		}
		return FALSE;
	}
	if( cnt == 1 ) {
		ADDRESSBOOK_addAddressBook( meindex,
                                    CONNECT_getTradecardcharaindex(fd,0) );
		return TRUE;
	}else if( cnt > 1 ) {
		int		strlength;
		char	msgbuf[1024];
		char	escapebuf[2048];
		strcpy( msgbuf, "1\n和誰交換名片呢？\n");
		strlength = strlen( msgbuf);
		for( i = 0;
             CONNECT_getTradecardcharaindex(fd,i) != -1 
			&& i< CONNECT_WINDOWBUFSIZE; i ++ ){
			char	*a = CHAR_getChar( CONNECT_getTradecardcharaindex(fd,i),
                                       CHAR_NAME);
			char	buf[256];
			snprintf( buf, sizeof( buf),"%s\n", a);
			if( strlength + strlen( buf) > arraysizeof( msgbuf)){
				print( "%s:%d視窗訊息buffer不足。\n",
						__FILE__,__LINE__);
				break;
			}
			strcpy( &msgbuf[strlength], buf);
			strlength += strlen(buf);
		}
		lssproto_WN_send( fd, WINDOW_MESSAGETYPE_SELECT, 
						WINDOW_BUTTONTYPE_CANCEL,
						CHAR_WINDOWTYPE_SELECTTRADECARD,
						-1,
					makeEscapeString( msgbuf, escapebuf, sizeof(escapebuf)));
		return TRUE;
	}

	return FALSE;
}

static int ADDRESSBOOK_findBlankEntry( int cindex )
{
	int i;
	
	if( CHAR_CHECKINDEX( cindex ) == FALSE )return -1;
	
	for( i=0 ; i<ADDRESSBOOK_MAX ; i++){
		ADDRESSBOOK_entry *ae;
		ae = CHAR_getAddressbookEntry( cindex , i );
		if( ae && ae->use == FALSE ) {
			return i;
		}
	}
	return -1;
}

static BOOL ADDRESSBOOK_makeEntryFromCharaindex( int charaindex,
												 ADDRESSBOOK_entry* ae)
{
	char *cdkey;

	if( !CHAR_CHECKINDEX(charaindex) ) return FALSE;

	memset( ae,0,sizeof(ADDRESSBOOK_entry) );
	cdkey = CHAR_getChar( charaindex, CHAR_CDKEY);
	if( cdkey == NULL ){
		print( "ADDRESSBOOK_makeEntryFromCharaindex:"
			   " strange! getcdkeyFromCharaIndex returns NULL!"
			   " charaindex: %d\n" , charaindex );
		return FALSE;
	}
	strcpysafe( ae->cdkey , sizeof( ae->cdkey ),cdkey);

	strcpysafe( ae->charname,sizeof( ae->charname),
				CHAR_getChar(charaindex,CHAR_NAME) );
	ae->level       = CHAR_getInt( charaindex , CHAR_LV );
	ae->duelpoint   = CHAR_getInt( charaindex, CHAR_DUELPOINT);
	ae->graphicsno  = CHAR_getInt( charaindex , CHAR_FACEIMAGENUMBER );
	ae->online = getServernumber();
	ae->use = TRUE;
	ae->transmigration = CHAR_getInt( charaindex, CHAR_TRANSMIGRATION);

	return TRUE;
}

void ADDRESSBOOK_notifyLoginLogout( int cindex , int flg )
{
	int i;
	char *cd=NULL;
	char *nm = CHAR_getChar( cindex , CHAR_NAME );
	int     playernum = CHAR_getPlayerMaxNum();
	
	if( !CHAR_CHECKINDEX( cindex ) )return;
	cd = CHAR_getChar( cindex, CHAR_CDKEY);

	CHAR_send_DpDBUpdate_AddressBook( cindex, flg );

	for( i = 0 ; i < playernum ; i++){
		if( CHAR_CHECKINDEX( i) && i != cindex ) {
			int j;
			for( j = 0 ; j<ADDRESSBOOK_MAX ; j++){
				ADDRESSBOOK_entry *ae;
				ae = CHAR_getAddressbookEntry( i , j );
				if( ae && ae->use == TRUE &&
					strcmp( ae->cdkey , cd ) == 0 &&
					strcmp( ae->charname, nm ) == 0 ){

					ae->online = (flg == 0 ) ? 0: getServernumber();
					ae->level = CHAR_getInt( cindex , CHAR_LV );
					ae->duelpoint = CHAR_getInt( cindex, CHAR_DUELPOINT);
					ae->graphicsno = CHAR_getInt( cindex, CHAR_FACEIMAGENUMBER );
					ae->transmigration = CHAR_getInt( cindex, CHAR_TRANSMIGRATION);
				
					ADDRESSBOOK_sendAddressbookTableOne( i,j );

					break;
				}
			}
		}
	}

	if( flg == 0 ){
		saacproto_Broadcast_send( acfd,cd, nm, "offline", 1);
	}else if( flg == 1 ) {
		for( i = 0 ; i < ADDRESSBOOK_MAX; i++ ){
			int j;
			ADDRESSBOOK_entry* ae;
			ae = CHAR_getAddressbookEntry( cindex, i );
			if( ae->use == 0 )continue;
			ae->online = 0;
			for( j=0 ; j < playernum ; j++ ) {
				if( CHAR_CHECKINDEX( j) &&
					strcmp( ae->cdkey, CHAR_getChar( j, CHAR_CDKEY)) == 0 &&
					strcmp( ae->charname, CHAR_getChar( j, CHAR_NAME) )== 0){
					ae->level = CHAR_getInt( j, CHAR_LV );
					ae->graphicsno = CHAR_getInt( j, CHAR_FACEIMAGENUMBER );
					ae->online = getServernumber();
					ae->transmigration = CHAR_getInt( j, CHAR_TRANSMIGRATION);
					break;
				}
			}
			if( j == playernum) {
#ifndef _DEATH_CONTEND
				char buff[512];
				char escapebuf[1024];
				ae->online = 0;
				snprintf( buff, sizeof(buff), "%s_%s", ae->cdkey, ae->charname);
				makeEscapeString( buff, escapebuf, sizeof(escapebuf));
				saacproto_DBGetEntryString_send( acfd, DB_ADDRESSBOOK, escapebuf, 0,0);
#endif
			}
		}
		ADDRESSBOOK_sendAddressbookTable(cindex);
		saacproto_Broadcast_send( acfd,cd, nm, "online", 1);
		saacproto_MessageFlush_send( acfd, cd, nm);
	}
}

BOOL ADDRESSBOOK_sendAddressbookTable( int cindex )
{
	int stringlen=0;
	int i;

	if( !CHAR_CHECKINDEX( cindex ) )return FALSE;
	
	for( i=0 ; i<ADDRESSBOOK_MAX ; i++){
		ADDRESSBOOK_entry *ae;
		ae = CHAR_getAddressbookEntry( cindex , i );
		if( ae && ae->use ){
			char tmp[CHARNAMELEN+32];
			char charname_escaped[CHARNAMELEN*2];
			makeEscapeString( ae->charname, charname_escaped ,
							  sizeof(charname_escaped  ));
			/*  銀迕白仿弘|  蟆|伊矛伙|仿奶白|白仿弘   */
			snprintf( tmp , sizeof( tmp ),
					  "%d|%s|%d|%d|%d|%d|%d|" ,
					  ae->use,
					  charname_escaped , ae->level , 
					  ae->duelpoint,ae->online,ae->graphicsno,
					  ae->transmigration);
			strcpysafe  ( ADDRESSBOOK_returnstring + stringlen ,
						  sizeof(ADDRESSBOOK_returnstring) - stringlen,
						  tmp );
			stringlen += strlen( tmp );
			if( stringlen >= sizeof(ADDRESSBOOK_returnstring) ) {
				break;
			}
		}else{
			/*銀勻化卅中犯□正手沖徇及心匹霜月  */
			char    tmp[32];
			snprintf( tmp , sizeof( tmp ), "|||||||"  );
			strcpysafe  ( ADDRESSBOOK_returnstring + stringlen ,
						  sizeof(ADDRESSBOOK_returnstring) - stringlen,
						  tmp );
			stringlen += strlen( tmp );
			if( stringlen >= sizeof(ADDRESSBOOK_returnstring)) {
				break;
			}
		}
	}
	
	dchop( ADDRESSBOOK_returnstring, "|" );

	{
		int fd;
		fd = getfdFromCharaIndex( cindex );
		if( fd == -1 ) return FALSE;
		lssproto_AB_send( fd, ADDRESSBOOK_returnstring );
	}
	return TRUE;
}

BOOL ADDRESSBOOK_sendAddressbookTableOne( int cindex, int num )
{
	int stringlen=0;
	ADDRESSBOOK_entry *ae;

	if( !CHAR_CHECKINDEX( cindex ) )return FALSE;
	if( num < 0 || num > ADDRESSBOOK_MAX) return FALSE;
	
	ae = CHAR_getAddressbookEntry( cindex , num );
	
	if( ae && ae->use ){
		char tmp[CHARNAMELEN+32];
		char charname_escaped[CHARNAMELEN*2];
		makeEscapeString( ae->charname, charname_escaped ,
						  sizeof(charname_escaped  ));
		snprintf( tmp , sizeof( tmp ),
				  "%d|%s|%d|%d|%d|%d|%d|" ,
				  ae->use,
				  charname_escaped , ae->level , 
				  ae->duelpoint,ae->online,ae->graphicsno,
				  ae->transmigration);
		strcpysafe  ( ADDRESSBOOK_returnstring + stringlen ,
					  sizeof(ADDRESSBOOK_returnstring) - stringlen,
					  tmp );
		stringlen += strlen( tmp );
		if( stringlen >= sizeof(ADDRESSBOOK_returnstring) ) {
			return FALSE;
		}
	}else{
		char    tmp[32];
		snprintf( tmp , sizeof( tmp ), "|||||||"  );
		strcpysafe  ( ADDRESSBOOK_returnstring + stringlen ,
					  sizeof(ADDRESSBOOK_returnstring) - stringlen,
					  tmp );
		stringlen += strlen( tmp );
		if( stringlen >= sizeof(ADDRESSBOOK_returnstring)) {
			return FALSE;
		}
	}

	{
		int fd;
		fd = getfdFromCharaIndex( cindex );
		if( fd == -1 ) return FALSE;
		lssproto_ABI_send( fd, num, ADDRESSBOOK_returnstring );
	}
	return TRUE;
}

/*------------------------------------------------------------
 * 夫午勾及失玉伊旦皮永弁巨件玄伉毛｝  儂  卞卅雲允〔
 * 仇木反平乓仿忡繡迕卅及匹弁仿奶失件玄卞霜耨允月方曰手懇割
 * 卅手及匹丐月  邰互丐月〔
 * 婁醒
 *  a   ADDRESSBOOK_entry*    儂  卞仄凶中厭瞻  尺及禾奶件正
 * 忒曰襖
 *  char *
 ------------------------------------------------------------*/
char *ADDRESSBOOK_makeAddressbookString( ADDRESSBOOK_entry *a )
{
	char work1[256], work2[256];

	if( a->use == 0 ){
		/* 塢巨件玄伉分勻凶日塢  儂   */
		ADDRESSBOOK_returnstring[0] = '\0';
		return ADDRESSBOOK_returnstring;
	}
	
	makeEscapeString( a->cdkey, work1, sizeof( work1 ));
	makeEscapeString( a->charname , work2 , sizeof( work2 ));
	snprintf( ADDRESSBOOK_returnstring,
			  sizeof(  ADDRESSBOOK_returnstring ),
			  "%s|%s|%d|%d|%d|%d",
			  work1, work2, a->level, a->duelpoint, a->graphicsno,a->transmigration);

	return ADDRESSBOOK_returnstring;

}

/*------------------------------------------------------------
 *   儂  祭今木化中月失玉伊旦皮永弁巨件玄伉毛｝厭瞻  卞卅雲允〔
 * 仇及瑛絆厭瞻  及use動陸及樹  反敦僬卞卅月〔
 * 婁醒
 *  in      char*                     儂  
 *  a       ADDRESSBOOK_entry*      犯□正毛璋戶月赭
 * 忒曰襖
 * 勾友卞TRUE
 ------------------------------------------------------------*/
BOOL ADDRESSBOOK_makeAddressbookEntry( char *in , ADDRESSBOOK_entry *a )
{
	char work1[256], work2[256] , work3[256] , work4[256],work5[256],work6[256];
	int ret;
	if( strlen( in ) == 0 ){
		memset( a,0,sizeof(ADDRESSBOOK_entry) );
		a->use = 0;
		return TRUE;
	}
	
	getStringFromIndexWithDelim( in, "|" , 1 , work1 , sizeof( work1 ));
	getStringFromIndexWithDelim( in, "|" , 2 , work2 , sizeof( work2 ));
	getStringFromIndexWithDelim( in, "|" , 3 , work3 , sizeof( work3 ));
	getStringFromIndexWithDelim( in, "|" , 4 , work4 , sizeof( work4 ));
	getStringFromIndexWithDelim( in, "|" , 5 , work5 , sizeof( work5 ));
	work6[0] = '\0';
	ret = getStringFromIndexWithDelim( in, "|" , 6 , work6 , sizeof( work6 ));
	if( ret == FALSE ) {
		a->transmigration = 0;
	}
	else {
		a->transmigration = atoi( work6);
	}
	a->use = 1;

	makeStringFromEscaped( work1 );
	makeStringFromEscaped( work2 );

	strcpysafe( a->cdkey , sizeof(a->cdkey) , work1 );
	strcpysafe( a->charname , sizeof(a->charname), work2 );

	a->level = atoi( work3 );
	a->duelpoint = atoi( work4 );
	a->graphicsno = atoi( work5 );
		
	return FALSE;
}

void ADDRESSBOOK_addAddressBook( int meindex, int toindex)
{
	char tmpstring[CHARNAMELEN +
				  ADDRESSBOOK_FIXEDMESSAGE_MAXLEN];

	int  hisblank;
	int	myblank;
	int	myaddindex, toaddindex;
	int	dir;
    char *cdkey;

	hisblank = ADDRESSBOOK_findBlankEntry( toindex );
	if( hisblank < 0 ) {
		CHAR_talkToCli( meindex, -1, ADDRESSBOOK_HISTABLEFULL, CHAR_COLORWHITE);
		return ;
	}
	myblank = ADDRESSBOOK_findBlankEntry( meindex );
	if( myblank < 0 ){
		CHAR_talkToCli( meindex , -1, ADDRESSBOOK_MYTABLEFULL, CHAR_COLORWHITE );
		return ;
	}

	cdkey = CHAR_getChar( toindex, CHAR_CDKEY);
	myaddindex = ADDRESSBOOK_getIndexInAddressbook( meindex, cdkey, 
								CHAR_getChar( toindex, CHAR_NAME));
	cdkey = CHAR_getChar( meindex, CHAR_CDKEY);
	toaddindex = ADDRESSBOOK_getIndexInAddressbook( toindex, cdkey, 
									CHAR_getChar(meindex, CHAR_NAME));
	if( myaddindex < 0 ){
		ADDRESSBOOK_entry   hisentry;
		
		if( ADDRESSBOOK_makeEntryFromCharaindex(toindex,&hisentry) == FALSE ){
			return ;
		}
		CHAR_setAddressbookEntry( meindex, myblank,&hisentry );

		snprintf( tmpstring, sizeof( tmpstring),
				  toaddindex < 0 ? ADDRESSBOOK_ADDED: ADDRESSBOOK_GIVEADDRESS, 
				  hisentry.charname);
		CHAR_talkToCli( meindex , -1,tmpstring, CHAR_COLORWHITE );
		ADDRESSBOOK_sendAddressbookTableOne( meindex, myblank);
	}else{
		snprintf( tmpstring, sizeof( tmpstring), ADDRESSBOOK_TAKEADDRESS1,
				  			CHAR_getChar( toindex, CHAR_NAME));
		CHAR_talkToCli( meindex, -1, tmpstring, CHAR_COLORWHITE);
			
	}
	if( toaddindex < 0  ) {
		ADDRESSBOOK_entry   meae;
		if( ADDRESSBOOK_makeEntryFromCharaindex(meindex,&meae) == FALSE) {
			return;
		}
		CHAR_setAddressbookEntry( toindex, hisblank,&meae );
		snprintf( tmpstring , sizeof( tmpstring),
				  myaddindex < 0 ? ADDRESSBOOK_ADDED : ADDRESSBOOK_GIVEADDRESS,
				  CHAR_getChar( meindex, CHAR_NAME ) );
		CHAR_talkToCli( toindex, -1, tmpstring, CHAR_COLORWHITE);
		ADDRESSBOOK_sendAddressbookTableOne( toindex , hisblank);
	}else {
		if( myaddindex < 0 ) {
		snprintf( tmpstring , sizeof( tmpstring),
					ADDRESSBOOK_TAKEADDRESS2,
				  CHAR_getChar( meindex, CHAR_NAME ) );
			CHAR_talkToCli( toindex, -1, tmpstring, CHAR_COLORWHITE);
		}
	}
	dir = NPC_Util_GetDirCharToChar( toindex, meindex, 0);
	if( dir != -1) {
		
		if( CHAR_getInt( toindex, CHAR_DIR) != dir) {
			CHAR_setInt( toindex, CHAR_DIR, dir);
		}
		CHAR_sendWatchEvent( CHAR_getWorkInt( toindex, CHAR_WORKOBJINDEX),
								CHAR_ACTNOD,NULL,0,TRUE);
		CHAR_setWorkInt( toindex, CHAR_WORKACTION, CHAR_ACTNOD);
		CHAR_sendWatchEvent( CHAR_getWorkInt( meindex, CHAR_WORKOBJINDEX),
								CHAR_ACTNOD,NULL,0,TRUE);
		CHAR_setWorkInt( meindex, CHAR_WORKACTION, CHAR_ACTNOD);
	}
	return;
}


void ADDRESSBOOK_DispatchMessage( char *cd, char *nm, char *value, int mode)
{
	int		i;
	char work[256];
	int	online,level,duelpoint, faceimagenumber, transmigration;
	int	playernum = CHAR_getPlayerMaxNum();
	int ret;

	getStringFromIndexWithDelim( value, "|" , 1 , work , sizeof( work ));
	online = atoi( work);
	getStringFromIndexWithDelim( value, "|" , 2 , work , sizeof( work ));
	level = atoi( work);
	getStringFromIndexWithDelim( value, "|" , 3 , work , sizeof( work ));
	duelpoint = atoi( work);
	getStringFromIndexWithDelim( value, "|" , 4 , work , sizeof( work ));
	faceimagenumber = atoi( work);
	ret = getStringFromIndexWithDelim( value, "|" , 5 , work , sizeof( work ));
	if( ret ) {
		transmigration = atoi( work);
	}
	else {
		transmigration = 0;
	}

	if( online == getServernumber()) {
		for( i = 0 ; i < playernum ; i++) {
			if( CHAR_CHECKINDEX( i )) {
				char *c = CHAR_getChar( i, CHAR_CDKEY);
				char *n = CHAR_getChar( i, CHAR_NAME);
				if( c == NULL || n == NULL ) continue;
				if( strcmp( c , cd ) == 0 && strcmp( n, nm ) == 0 )  {
					break;
				}
			}
		}
		if( i == playernum ) {
			online = 0;
			saacproto_Broadcast_send( acfd,cd, nm, "offline", 1);
		}
	}

	for( i = 0 ; i < playernum ; i++) {
		if( CHAR_CHECKINDEX( i )) {
			int j;
			for( j = 0 ; j < ADDRESSBOOK_MAX ; j++) {
				ADDRESSBOOK_entry *ae;
				ae = CHAR_getAddressbookEntry( i ,j );
				if( ae && ae->use == TRUE &&
					strcmp( ae->cdkey , cd ) == 0 &&
					strcmp( ae->charname, nm ) == 0 )
				{
					if( mode == 0 ) {
						ae->use = FALSE;
					}else {
						ae->online = online;
						ae->level = level;
						ae->duelpoint = duelpoint;
						ae->graphicsno = faceimagenumber;
						ae->transmigration = transmigration;
					}
					ADDRESSBOOK_sendAddressbookTableOne( i,j );

					break;
				}
			}
		}
	}
}


BOOL ADDRESSBOOK_AutoaddAddressBook( int meindex, int toindex)
{
	int  hisblank;
	int	myblank;
	int	myaddindex, toaddindex;

	char *cdkey;

	myblank = ADDRESSBOOK_findBlankEntry( meindex );
	hisblank = ADDRESSBOOK_findBlankEntry( toindex );
	if( hisblank < 0 || myblank < 0) { //"名片匣已滿。"
			return FALSE;
	}
	
	cdkey = CHAR_getChar( toindex, CHAR_CDKEY);
	myaddindex = ADDRESSBOOK_getIndexInAddressbook( meindex, cdkey, 
								CHAR_getChar( toindex, CHAR_NAME));
	cdkey = CHAR_getChar( meindex, CHAR_CDKEY);
	toaddindex = ADDRESSBOOK_getIndexInAddressbook( toindex, cdkey, 
									CHAR_getChar(meindex, CHAR_NAME));
	if( myaddindex < 0 ){
		ADDRESSBOOK_entry   meae;
		ADDRESSBOOK_entry   hisentry;
		if( ADDRESSBOOK_makeEntryFromCharaindex(toindex,&hisentry) == FALSE ||
			ADDRESSBOOK_makeEntryFromCharaindex(meindex,&meae) == FALSE ){
			return FALSE;
		}
		CHAR_setAddressbookEntry( meindex, myblank,&hisentry );
		CHAR_setAddressbookEntry( toindex, hisblank,&meae );
		ADDRESSBOOK_sendAddressbookTableOne( meindex, myblank);
		ADDRESSBOOK_sendAddressbookTableOne( toindex , hisblank);
	}
	return TRUE;
}


#include "version.h"
#include <string.h>
#include "char.h"
#include "object.h"
#include "char_base.h"
#include "npcutil.h"
#include "configfile.h"
#include "lssproto_serv.h"
#include "saacproto_cli.h"
#include "npc_familyman.h"
#include "family.h"
#include "handletime.h"

extern	int	familyNumTotal;
extern	char	familyListBuf[MAXFAMILYLIST]; 

char sendbuf[1024];
char buf[1024];
char subbuf[128];
int i, j;

/* 
 * 澀爛今木凶它奴件玉它毛請允NPC
 * 棵啞  平旦玄失玉矛件民乓□仁日中卅日綜木月井手〔
 *
 */
 
struct	{
	int		windowno;
	int		windowtype;
	int		buttontype;
	int		takeitem;
	int		giveitem;
	char	message[4096];
}w;

struct	{
	BOOL	use;
	int		checkhaveitem;
	int		checkhaveitemgotowin;
	int		checkdonthaveitem;
	int		checkdonthaveitemgotowin;
	int		warp;
	int		battle;
	int		gotowin;
}buttonproc[13];		/* ok,cancel, yes,no,prev,next 及凜及質   */





enum {
	CHAR_WORK_MSGCOLOR	= CHAR_NPCWORKINT1,
};
static void NPC_Familyman_selectWindow( int meindex, int toindex, int num);
static BOOL NPC_Familyman_readData( int meindex, int windowno, BOOL chkflg);
static int NPC_Familyman_restoreButtontype( char *data );

/*********************************
* 賡渝質  
*********************************/
BOOL NPC_FamilymanInit( int meindex )
{
	char	argstr1[NPC_UTIL_GETARGSTR_BUFSIZE];
	char *argstr;
	char	buf[1024];
	//int		i;
	//char	secondToken[1024];
	
	// Robin test
	// print(" familyman_Init ");	
	
	argstr = NPC_Util_GetArgStr( meindex, argstr1, sizeof( argstr1));
	
	if( NPC_Util_GetStrFromStrWithDelim( argstr, "conff", buf, sizeof( buf))
		== NULL ) 
	{
		print( "familyman:沒有指定設定的檔案 。\n");
		return FALSE;
	}
	/* 賡渝凜卞澀爛犯□正毛民尼永弁仄化支月 */
	if( !NPC_Familyman_readData( meindex, -1, TRUE) ) {
		return FALSE;
	}
	
	CHAR_setInt( meindex , CHAR_WHICHTYPE , CHAR_TYPEWINDOWMAN );
	
	return TRUE;
}




/*********************************
*   仄井仃日木凶凜及質  
*********************************/
void NPC_FamilymanTalked( int meindex , int talkerindex , char *szMes ,int color )
{

	NPC_Familyman_selectWindow( meindex, talkerindex, 1 );
	
}
/*********************************
* 葦日木凶凜及質  
*********************************/
void NPC_FamilymanLooked( int meindex , int lookedindex)
{

	print(" Look:me:%d,looked:%d ",meindex,lookedindex);
	NPC_Familyman_selectWindow( meindex, lookedindex,1 );
	
}

static void NPC_Familyman_selectWindow( int meindex, int toindex, int num)
{

	int		fd;
	char	buf[256];
	
	/* 皿伊奶乩□卞覆仄化分仃  殺允月 */
	if( CHAR_getInt( toindex , CHAR_WHICHTYPE ) != CHAR_TYPEPLAYER ) {
		return;
	}
	/* ㄠ弘伉永玉動  及心 */
	if( !NPC_Util_charIsInFrontOfChar( toindex, meindex, 1 )) return; 

	if( !NPC_Familyman_readData( meindex, num, FALSE) ) {
		print( "familyman:readdata error\n");
		return;
	}
	
	fd = getfdFromCharaIndex( toindex);
	if( fd != -1 ) {
		lssproto_WN_send( fd, w.windowtype, 
						w.buttontype,
						w.windowno+100,
						CHAR_getWorkInt( meindex, CHAR_WORKOBJINDEX),
						makeEscapeString( w.message, buf, sizeof(buf)));

	}
}

void NPC_FamilymanWindowTalked( int meindex, int talkerindex, 
								int seqno, int select, char *data)

{
		
	int		button = -1;
	char	buf[256];

	/* ㄠ弘伉永玉動  及心 */
	if( NPC_Util_CharDistance( talkerindex, meindex ) > 1) return;
	
	// CoolFish Add For Check Old Leader Del Family
	if(seqno == CHAR_WINDOWTYPE_FAMILYMAN_OUT)
	{
		int fd = getfdFromCharaIndex( talkerindex);
		if(select == WINDOW_BUTTONTYPE_YES)
			FAMILY_Leave( fd, talkerindex, "E|1");
		return;
	}
	// CoolFish End

	/* 仇及它奴件玉它  寞及犯□正毛  心  戈 */	
	if( !NPC_Familyman_readData( meindex, seqno - 100, FALSE) ) {
		print( "familyman:readdata error\n");
		return;
	}
	/* 瓷仄凶示正件毛譬屯月 */
	if( w.windowtype == WINDOW_MESSAGETYPE_SELECT ) {
		button = atoi( data)+5;
		if( button > 12 ) {
			print( "familyman:invalid button[%d]\n", button);
			return;
		}
	}
	else if( select & WINDOW_BUTTONTYPE_OK) button = 0;
	else if( select & WINDOW_BUTTONTYPE_CANCEL) button = 1;
	else if( select & WINDOW_BUTTONTYPE_YES) button = 2;
	else if( select & WINDOW_BUTTONTYPE_NO) button = 3;
	else if( select & WINDOW_BUTTONTYPE_PREV) button = 4;
	else if( select & WINDOW_BUTTONTYPE_NEXT) button = 5;
	else {
		print( "familyman:invalid button[%d]\n", select);
		return;
	}
	if( buttonproc[button].use == TRUE ) {
		int	fd;
		int	newwin = -1;
	
		fd = getfdFromCharaIndex( talkerindex);
		
		/* 示正件卞方勻化質  毛孔曰歹仃月 */
		if( newwin == -1 ) {
			newwin = buttonproc[button].gotowin;
		}
		
		// Robin
		// 成立
		if( newwin == 5 )	{
			if( CHAR_getInt( talkerindex, CHAR_FMINDEX ) > 0 )
			{
				//CHAR_talkToCli( talkerindex, -1, "資格不符！已經加入家族。", CHAR_COLORWHITE );
				lssproto_WN_send( fd, WINDOW_MESSAGETYPE_MESSAGE,
					WINDOW_BUTTONTYPE_OK, -1, -1,
					makeEscapeString( "\n很抱歉喔！你已經加入家族了！", buf, sizeof(buf)));
				return;
			}		
			if( (CHAR_getInt( talkerindex, CHAR_TRANSMIGRATION ) == 0)
				&& (CHAR_getInt( talkerindex, CHAR_LV) < 30)  )
			{
				//CHAR_talkToCli( talkerindex, -1, "很抱歉！等級不足。", CHAR_COLORWHITE );
				lssproto_WN_send( fd, WINDOW_MESSAGETYPE_MESSAGE,
					WINDOW_BUTTONTYPE_OK, -1, -1,
					makeEscapeString( "\n很抱歉喔！你的等級不足！", buf, sizeof(buf)));
				return;
                        }
                        
                        if( !NPC_EventCheckFlg( talkerindex, 4 ) )
                        {
				lssproto_WN_send( fd, WINDOW_MESSAGETYPE_MESSAGE,
					WINDOW_BUTTONTYPE_OK, -1, -1,
					makeEscapeString( "\n很抱歉喔！你必須先完成成人禮才行！", buf, sizeof(buf)));
				return;
                        }
                        
			if( CHAR_getInt( talkerindex, CHAR_GOLD ) < 10000 )
			{
				lssproto_WN_send( fd, WINDOW_MESSAGETYPE_MESSAGE,
					WINDOW_BUTTONTYPE_OK, -1, -1,
					makeEscapeString( "\n很抱歉喔！你的申請手續費不足！", buf, sizeof(buf)));
				return;
			}
						
			lssproto_WN_send( fd, WINDOW_MESSAGETYPE_FAMILYADD, WINDOW_BUTTONTYPE_NONE, CHAR_WINDOWTYPE_FAMILYMAN_ADD,
				CHAR_getWorkInt( meindex, CHAR_WORKOBJINDEX), "Hello!!" );
			
			return;
		}
		// 列表 , 加入
		if( newwin == 6 )
		{
			/*
			strcpy( buf, "");
			j = 0;
			for( i=1 ; i<=8 ; i++  ) {
				if( i > familyNumTotal )	break;
				if( getStringFromIndexWithDelim( familyListBuf, "|", i, subbuf,
				        sizeof(subbuf) ) == FALSE)	break;
				strcat( buf, "|" );
				strcat( buf, subbuf );
				j++;
			}

			sprintf( sendbuf, "S|F|%d|%d|%d%s", familyNumTotal, 1, j, buf );
			//print(" FL:%s ", sendbuf );			
			lssproto_FM_send( fd, sendbuf );
			*/
			
			//saacproto_ACShowFMList_send( acfd );
			int JZ_FMTimeLimit;
   		char buf1[1024];
   		int Timed,Timeh,Timem;
   		JZ_FMTimeLimit = CHAR_getInt( talkerindex , CHAR_FMTIMELIMIT );
   		if (NowTime.tv_sec < JZ_FMTimeLimit){
   			JZ_FMTimeLimit = JZ_FMTimeLimit-NowTime.tv_sec;
   		
      	Timed = JZ_FMTimeLimit / 86400;
      	Timeh = JZ_FMTimeLimit / 3600 - Timed*24;
      	Timem = JZ_FMTimeLimit / 60 - Timed*24*60 - Timeh*60;
   			sprintf( buf1 ,"\n還有%d天%d時%d分，您才能重新加入家族",Timed,Timeh,Timem);
   			lssproto_WN_send( fd, WINDOW_MESSAGETYPE_MESSAGE,
					WINDOW_BUTTONTYPE_OK,
					-1, -1,
					makeEscapeString( buf1, buf, sizeof(buf)));
				return;
   		}else{
				FAMILY_Detail( fd, talkerindex, "S|F|1|0" );			
				return;
			}
		}
		// 退出
		if( newwin == 7 )
		{
			if( CHAR_getInt( talkerindex, CHAR_FMINDEX ) == -1 ) {
				// CHAR_talkToCli( talkerindex, -1, "你還未加入任何家族呀。", CHAR_COLORWHITE );
				lssproto_WN_send( fd, WINDOW_MESSAGETYPE_MESSAGE,
					WINDOW_BUTTONTYPE_OK, -1, -1,
					makeEscapeString( "\n很抱歉喔！你還沒有加入任何家族呀！", buf, sizeof(buf)));
				return;
			}
						
			//if (CHAR_getInt(talkerindex, CHAR_FMLEADERFLAG) == FMMEMBER_LEADER) {
			//	CHAR_talkToCli( talkerindex, -1, "族長....", CHAR_COLORWHITE );
			//	return;
			//}
			
			print(" WN2FM ");

			// CoolFish Change for old leader del family check
			if(CHAR_getInt(talkerindex, CHAR_FMLEADERFLAG) == FMMEMBER_LEADER)
			{
				lssproto_WN_send(fd, WINDOW_MESSAGETYPE_MESSAGE,
					WINDOW_BUTTONTYPE_YESNO, CHAR_WINDOWTYPE_FAMILYMAN_OUT,
					CHAR_getWorkInt(meindex, CHAR_WORKOBJINDEX),
					makeEscapeString("\n您現在是這個家族的族長喔...\n\n家族解散了就無法再救回唷！～\n\n確定要解散家族嗎？",buf, sizeof(buf)));		
				return;
			}
			else
				FAMILY_Leave( fd, talkerindex, "E|1");
			// CoolFish Change End
									
			return;
		}
		
		if( !NPC_Familyman_readData( meindex, newwin, FALSE) ) {
			print( "familyman:readdata error\n");
			return;
		}
		
		//fd = getfdFromCharaIndex( talkerindex);
		if( fd != -1 ) {
			lssproto_WN_send( fd, w.windowtype, 
							w.buttontype,
							w.windowno+100,
							CHAR_getWorkInt( meindex, CHAR_WORKOBJINDEX),
							makeEscapeString( w.message, buf, sizeof(buf)));
		}
		
		
		
	}
}
/* 
 * 澀爛白央奶伙毛  氏匹隙爛今木凶windowno及犯□正毛本永玄允月
 * 
 * 婁醒「
 *		meindex		int		仇及NPC及charaindex
 *		windowno	int		它奴件玉它  寞
 *		
 */
static BOOL NPC_Familyman_readData( int meindex, int windowno, BOOL chkflg)
{
	
	int		i;
	int		linenum = 0;
	int		endflg = FALSE;
	int		buttonendflg;
	int		winno = -1;
	int		buttonconfmode;
	int		b_mode;
	int		selectnum ;
	int		messagepos;
	BOOL	errflg = FALSE;
	BOOL	readflg = TRUE;
	FILE	*fp;
	char	argstr1[NPC_UTIL_GETARGSTR_BUFSIZE];
	char *argstr;
	char	filename[64];
	char	opfile[128];
	char	line[1024];
	char	firstToken[1024];
	char	secondToken[1024];
	
	/* 它奴件玉它及澀爛毛  曰  戈厭瞻   */
	
	argstr = NPC_Util_GetArgStr( meindex, argstr1, sizeof( argstr1));
	/* 澀爛白央奶伙  潸   */
	NPC_Util_GetStrFromStrWithDelim( argstr, "conff", filename, sizeof( filename));

	sprintf( opfile, "%s/", getNpcdir( ) );
	strcat( opfile, filename);
	
	fp = fopen( opfile, "r");
	if( fp == NULL ) {
		print( "familyman:file open error [%s]\n", opfile);
		return FALSE;
	}
	
	while( readflg == TRUE ) {
		endflg = FALSE;
		buttonendflg = TRUE;
		buttonconfmode = FALSE;
		selectnum = 0;
		messagepos = 0;
		winno = -1;
		b_mode = -1;
		errflg = FALSE;

		/* 賡渝祭 */
		w.windowno = -1;
		w.windowtype = -1;
		w.buttontype = -1;
		w.takeitem = -1;
		w.giveitem = -1;
		w.message[0] = '\0';
	
		for( i = 0; i < arraysizeof( buttonproc); i ++ ) {
			buttonproc[i].use = FALSE;
			buttonproc[i].checkhaveitem = -1;
			buttonproc[i].checkhaveitemgotowin = -1;
			buttonproc[i].checkdonthaveitem = -1;
			buttonproc[i].checkdonthaveitemgotowin = -1;
			buttonproc[i].warp = -1;
			buttonproc[i].battle = -1;
			buttonproc[i].gotowin = -1;
		}

		while( 1) {
			char    buf[256];
			int		ret;
			if( !fgets( line, sizeof( line), fp)){
				readflg = FALSE;
				break;
			}
			
			linenum ++;
			
			/* 戊丟件玄反  骰 */
			if( line[0] == '#' || line[0] == '\n') continue;
			/* 荼墊潸月 */
			chomp( line );
			
			/*  墊毛幫溥允月    */
			/*  引內 tab 毛 " " 卞  五晶尹月    */
			replaceString( line, '\t' , ' ' );
			/* 燮  及旦矢□旦毛潸月〔*/
			for( i = 0; i < strlen( line); i ++) {
				if( line[i] != ' ' ) {
					break;
				}
				strcpy( buf, &line[i]);
			}
			if( i != 0 ) strcpy( line, buf);

			/* delim "=" 匹  賡(1)及玄□弁件毛  月*/
			ret = getStringFromIndexWithDelim( line, "=",  1, firstToken,
											   sizeof( firstToken ) );
			if( ret == FALSE ){
				print( "Find error at %s in line %d. Ignore\n",
					   filename , linenum);
				continue;
			}
			/* delim "=" 匹2    及玄□弁件毛  月*/
			ret = getStringFromIndexWithDelim( line, "=", 2, secondToken,
											   sizeof( secondToken ) );
			if( ret == FALSE ){
				print( "Find error at %s in line %d. Ignore\n",
					   filename , linenum);
				continue;
			}
			
			if( strcasecmp( firstToken, "winno") == 0 ) {
				if( winno != -1 ) {
					print( "familyman:已有winno卻重新定義winno\n");
					print( "filename:[%s] line[%d]\n", filename, linenum);
					errflg = TRUE;
					readflg = FALSE;
					break;
				}
				/* 它奴件玉它No毛忡繡 */
				winno = atoi( secondToken);
				continue;
			}
			/* 它奴件玉它No 互瑁引勻化中卅中凜及墊反  骰允月 */
			if( winno == -1 ) {
				print( "familyman:winno 尚未定義，資料卻已設定。\n");
				print( "filename:[%s] line[%d]\n", filename, linenum);
				readflg = FALSE;
				errflg = FALSE;
				break;
			}
			/* 它奴件玉它No 互域譙仄凶凜反橢瘀毛  戈〔
			 * 公木動陸反  骰允月 */
			if( (chkflg == FALSE && winno == windowno )||
				chkflg == TRUE) 
			{
				if( buttonconfmode == TRUE ) {
					if( strcasecmp( firstToken, "gotowin") == 0 ) {
						buttonproc[b_mode].gotowin = atoi( secondToken);
					}
					else if( strcasecmp( firstToken, "checkhaveitem") == 0 ) {
						buttonproc[b_mode].checkhaveitem = atoi( secondToken);
					}
					else if( strcasecmp( firstToken, "haveitemgotowin") == 0 ) {
						buttonproc[b_mode].checkhaveitemgotowin = atoi( secondToken);
					}
					else if( strcasecmp( firstToken, "checkdonthaveitem") == 0 ) {
						buttonproc[b_mode].checkdonthaveitem = atoi( secondToken);
					}
					else if( strcasecmp( firstToken, "donthaveitemgotowin") == 0 ) {
						buttonproc[b_mode].checkdonthaveitemgotowin = atoi( secondToken);
					}
					else if( strcasecmp( firstToken, "endbutton") == 0 ) {
						if( buttonproc[b_mode].gotowin == - 1 ) {
							if( buttonproc[b_mode].checkhaveitem == -1 && 
								buttonproc[b_mode].checkdonthaveitem == -1)
							{
								errflg = TRUE;
							}
							else {
								/* 升勻切井井凶勻吊分仃匹手澀爛今木化中木壬     */
								if( !((buttonproc[b_mode].checkhaveitem != -1 && 
									   buttonproc[b_mode].checkhaveitemgotowin != -1)
									 || (buttonproc[b_mode].checkdonthaveitem != -1 && 
									     buttonproc[b_mode].checkdonthaveitemgotowin != -1)))
								{
									errflg = TRUE;
								}
							}
						}
						
						if( errflg == TRUE) {
							print( "familyman: 找不到gotowin\n");
							print( "filename:[%s] line[%d]\n", filename, linenum);
							readflg = FALSE;
							errflg = TRUE;
							break;
						}
						buttonproc[b_mode].use = TRUE;
						buttonconfmode = FALSE;
						buttonendflg = TRUE;
					}
				}
				else {
					
					w.windowno = winno;
					/* 它奴件玉它正奶皿及澀爛 */
					if( strcasecmp( firstToken, "wintype") == 0 ) {
						w.windowtype = atoi( secondToken);
					}
					/* 示正件正奶皿及澀爛 */
					else if( strcasecmp( firstToken, "buttontype") == 0 ) {
						w.buttontype = NPC_Familyman_restoreButtontype( secondToken);
					}
					/* getitem及澀爛 */
					else if( strcasecmp( firstToken, "takeitem") == 0 ) {
						w.takeitem = atoi( secondToken);
					}
					/* giveitem及澀爛 */
					else if( strcasecmp( firstToken, "giveitem") == 0 ) {
						w.giveitem = atoi( secondToken);
					}
					/* message及澀爛 */
					else if( strcasecmp( firstToken, "message") == 0 ) {
						if( messagepos == 0 ) {
							strcpy(  w.message, secondToken);
							messagepos = strlen( w.message);
						}
						else {
							w.message[messagepos]='\n';
							messagepos++;
							strcpy( &w.message[messagepos], secondToken);
							messagepos+=strlen(secondToken);
						}
					}
					/* 示正件毛瓷仄凶凜及澀爛 */
					else if( strcasecmp( firstToken, "okpressed") == 0 ) {
						buttonconfmode = TRUE;
						b_mode = 0;
						buttonendflg = FALSE;
					}
					else if( strcasecmp( firstToken, "cancelpressed") == 0 ) {
						buttonconfmode = TRUE;
						b_mode = 1;
						buttonendflg = FALSE;
					}
					else if( strcasecmp( firstToken, "yespressed") == 0 ) {
						buttonconfmode = TRUE;
						b_mode = 2;
						buttonendflg = FALSE;
					}
					else if( strcasecmp( firstToken, "nopressed") == 0 ) {
						buttonconfmode = TRUE;
						b_mode = 3;
						buttonendflg = FALSE;
					}
					else if( strcasecmp( firstToken, "prevpressed") == 0 ) {
						buttonconfmode = TRUE;
						b_mode = 4;
						buttonendflg = FALSE;
					}
					else if( strcasecmp( firstToken, "nextpressed") == 0 ) {
						buttonconfmode = TRUE;
						b_mode = 5;
						buttonendflg = FALSE;
					}
					else if( strcasecmp( firstToken, "selected") == 0 ) {
						buttonconfmode = TRUE;
						b_mode = 6 + selectnum;
						buttonendflg = FALSE;
						selectnum ++;
					}
					/* 澀爛蔽歹曰 */
					else if( strcasecmp( firstToken, "endwin") == 0 ) {
						endflg = TRUE;
						if( chkflg == FALSE) {
							readflg = FALSE;
						}
						break;
					}
					else {
						print( "familyman:設定是不可能的參數\n");
						print( "filename:[%s] line[%d]\n", filename, linenum);
					}
				}
			}
			else {
				if( strcasecmp( firstToken, "endwin") == 0 ) {
					winno = -1;
				}
			}
		}
		if( buttonendflg == FALSE) {
			print( "familyman: 找不到endbutton\n");
			print( "filename:[%s] line[%d]\n", filename, linenum);
			errflg = TRUE;
			break;
		}
		if( winno != -1 ) {
			if( w.windowtype == -1 ) {
				print( "familyman: 找不到wintype\n");
				print( "filename:[%s] line[%d]\n", filename, linenum);
				errflg = TRUE;
				break;
			}
			if( w.buttontype == -1 ) {
				print( "familyman: 找不到button\n");
				print( "filename:[%s] line[%d]\n", filename, linenum);
				errflg = TRUE;
				break;
			}
			if( strlen( w.message) == 0 ) {
				print( "familyman: 找不到message\n");
				print( "filename:[%s] line[%d]\n", filename, linenum);
				errflg = TRUE;
				break;
			}
		}
	}
	fclose( fp);
	
	if( chkflg == FALSE && w.windowno == -1 ) {
		print( "familyman: 找不到所指定的windowno\n");
		print( "filename:[%s] line[%d]\n", filename, linenum);
		return FALSE;
	}
	if( winno != -1 && endflg == FALSE) {
		print( "familyman: 找不到endwin\n");
		print( "filename:[%s] line[%d]\n", filename, linenum);
		return FALSE;
	}
	if( errflg == TRUE) return FALSE;
	
	return TRUE;
}
/*
 * buttontype=匹隙爛仄凶  儂  毛醒襖卞  晶允月〔
 *
 */
static int NPC_Familyman_restoreButtontype( char *data )
{
	int		ret = 0;
	int		rc;
	int		i;
	char	buff[1024];
	
	for( i = 1; ; i ++ ) {
		rc = getStringFromIndexWithDelim( data, "|",  i, buff,
											   sizeof( buff ) );
		if( rc == FALSE) break;
		if( strcasecmp( buff, "ok") == 0 ) {
			ret |= WINDOW_BUTTONTYPE_OK;
		}
		else if( strcasecmp( buff, "cancel") == 0 ) {
			ret |= WINDOW_BUTTONTYPE_CANCEL;
		}
		else if( strcasecmp( buff, "yes") == 0 ) {
			ret |= WINDOW_BUTTONTYPE_YES;
		}
		else if( strcasecmp( buff, "no") == 0 ) {
			ret |= WINDOW_BUTTONTYPE_NO;
		}
		else if( strcasecmp( buff, "prev") == 0 ) {
			ret |= WINDOW_BUTTONTYPE_PREV;
		}
		else if( strcasecmp( buff, "next") == 0 ) {
			ret |= WINDOW_BUTTONTYPE_NEXT;
		}
	}
	if( ret == 0 ) {
		ret = atoi( data);
	}
	return ret;
}



#include "version.h"
#include "common.h"
#include "readmap.h"
#include "char.h"
#include "map_util.h"
#include "lssproto_serv.h"
#include "net.h"
#include "anim_tbl.h"

/*#define MAPEDITORTROUBLE*/
#define SPR_kmydamY CG_HIT_MARK_10 // anim_tbl.h 卞瓚  今木月引匹  及巨白尼弁玄

/*------------------------------------------------------------
 * 洶仃月井升丹井  蠅允月〔平乓仿互  氏匹中月井升丹井婁醒互丐月〔
 * 婁醒
 *  ff          int     floor
 *  fx          int     x甄  
 *  fy          int     y甄  
 *  isfly       int       氏匹中月井升丹井
 * 忒曰襖
 *  洶仃月      TRUE(1)
 *  洶仃卅中    FALSE(0)
 ------------------------------------------------------------*/
BOOL MAP_walkAbleFromPoint( int ff, int fx, int fy, BOOL isfly )
{
    int map[2];

    if( !MAP_getTileAndObjData( ff,fx,fy, &map[0], &map[1] ) ){
        return FALSE;
	}

    if( isfly ){
        int i;
        for( i = 0 ; i < 2 ; i ++ )
            if( MAP_getImageInt( map[i], MAP_HAVEHEIGHT ) == TRUE  ){
                return FALSE;
			}
        return TRUE;
    }else{
        switch( MAP_getImageInt( map[1], MAP_WALKABLE ) ){
        case 0:
            return FALSE;
            break;
        case 1:
            if( MAP_getImageInt( map[0], MAP_WALKABLE ) == 1 ){
                return TRUE;
			}else{
                return FALSE;
			}
            break;
        case 2:
            return TRUE;
            break;
        default:
            return FALSE;
            break;
        }
    }

    return FALSE;
}



/*------------------------------------------------------------
 * 洶仃月井升丹井  蠅允月〔
 * 婁醒
 *  index       int     Char    匹及 Index
 *  ff          int     floor id
 *  fx          int     x甄  
 *  fy          int     y甄  
 * 忒曰襖
 *  洶中化方中      TRUE(1)
 *  洶中化反母丟    FALSE(0)
 ------------------------------------------------------------*/
BOOL MAP_walkAble( int index,int ff, int fx, int fy)
{
    /*      平乓仿卅日OK    */
    if( CHAR_getFlg( index, CHAR_ISTRANSPARENT ) )      return TRUE;

    // Arminius 7.9 Airplane ok
    if( CHAR_getInt( index, CHAR_WHICHTYPE ) == CHAR_TYPEBUS) return TRUE;
    if( CHAR_getWorkInt( index, CHAR_WORKPARTYMODE) == CHAR_PARTY_CLIENT ) {
      int oyaindex = CHAR_getWorkInt( index, CHAR_WORKPARTYINDEX1);
      if( CHAR_CHECKINDEX( oyaindex)) {
        if (CHAR_getInt( oyaindex, CHAR_WHICHTYPE ) == CHAR_TYPEBUS) return TRUE;
      }
    }

    return MAP_walkAbleFromPoint( ff,fx,fy,CHAR_getFlg(index,CHAR_ISFLYING)|CHAR_getWorkInt(index,CHAR_WORKSKYWALKER) );
}

/*------------------------------------------------------------
 * 甄  毛隙爛仄化｝公仇及左皮斥尼弁玄卞嫖今互丐月井升丹井
 * 婁醒
 *  fl              int     白夫失
 *  x               int     x 甄  
 *  y               int     y 甄  
 * 忒曰襖
 *  嫖今互丐月  TRUE
 *  嫖今互卅中  FALSE
 ------------------------------------------------------------*/
BOOL MAP_haveHeight( int fl, int x, int y )
{
    int     map[2];

    /*  穴永皿犯□正毛  化仁月  */
    if( !MAP_getTileAndObjData( fl,x,y, &map[0], &map[1] ) )
        return FALSE;

    return MAP_getImageInt( map[1], MAP_HAVEHEIGHT );
}



/*----------------------------------------
 * 旦  □正旦  祭煙及  凳毛允月
 * 婁醒
 *  index   int     平乓仿及奶件犯永弁旦
 *  map     int     穴永皿犯□正
 *  outof   BOOL    褡戈凜｝轉仁凜井 TRUE 卅日壬 褡戈凜匹丐月〔
 * 忒曰襖
 *  由仿丟□正  祭毛仄凶    TRUE
 *  由仿丟□正  祭毛仄卅井勻凶  FALSE
  ----------------------------------------*/
static BOOL MAP_changeCharStatusFromMapDataAndTime( int index,
                                                    int map, BOOL outof)
{
#if 0
// 旦玄□件巨奶斥匹反銀歹卅中
    int i;
    int offset;
    BOOL    change=FALSE;
    static struct tagStatusInteractionOfBitAndDefine{
        int     mapdataindex;
        int     charadataindex;
    }statusInteraction[]={
        {MAP_INTOPOISON,        CHAR_POISON},
        {MAP_INTOPARALYSIS,     CHAR_PARALYSIS},
        {MAP_INTOSILENCE,       CHAR_SLEEP},
        {MAP_INTOSTONE,         CHAR_STONE},
        {MAP_INTODARKNESS,      CHAR_DRUNK},
        {MAP_INTOCONFUSION,     CHAR_CONFUSION},

        {MAP_OUTOFPOISON,       CHAR_POISON},
        {MAP_OUTOFPARALYSIS,    CHAR_PARALYSIS},
        {MAP_OUTOFSILENCE,      CHAR_SLEEP},
        {MAP_OUTOFSTONE,        CHAR_STONE},
        {MAP_OUTOFDARKNESS,     CHAR_DRUNK},
        {MAP_OUTOFCONFUSION,    CHAR_CONFUSION},
    };

    if( outof == FALSE )offset = 6;
    else                offset= 0;
    int     newdata;
    for( i = 0 ; i < 6 ; i ++ ){
        newdata = MAP_getImageInt(map,statusInteraction[i+offset].
                                  mapdataindex);
        if( newdata > 0 ){
            change=TRUE;
            /*  旦  □正旦及澀爛    */
            CHAR_setInt( index,
                         statusInteraction[i+offset].charadataindex,
                         CHAR_getInt(index,
                                     statusInteraction[i+offset].
                                     charadataindex ) + newdata );
        }
    }
    return change;
#else
	return FALSE;
#endif
}


/*------------------------------------------------------------
 * Map 奶矛件玄及 pre post毛質  允月
 * 婁醒
 *  index       int     曇欠丹午仄化中月平乓仿及奶件犯永弁旦
 *  mode        BOOL    TRUE及凜反｝in  FALSE及凜反 out 卞覆殺
 * 忒曰襖卅仄
 ------------------------------------------------------------*/
static void MAP_dealprepostevent( int index, BOOL mode )
{
    int     map[2];
    int     i;
    int     damaged=FALSE,statuschange=FALSE;

    if( CHAR_getFlg(index,CHAR_ISFLYING) )
        /*    氏匹月支勾卞反窒手仄卅中  */
        return;

    if( !MAP_getMapDataFromCharIndex( index  ,  map ) ) return;
    /*  damage */
    int damage;
    for( i = 0 ; i < 2 ; i ++ ){
        damage = MAP_getImageInt(map[i],
                                 mode ? MAP_INTODAMAGE : MAP_OUTOFDAMAGE);
        if( damage != 0 ){
            /*  母丟□斥洶毛懇匹  蜇允月及匹  寞  木贅尹    */
            int opt[2]={SPR_kmydamY,-damage};
            damaged=TRUE;
            CHAR_setInt(index, CHAR_HP,
                        CHAR_getInt(index,CHAR_HP) + damage );
            CHAR_complianceParameter(index);
            /*  母丟□斥熬仃凶巨白尼弁玄請允    */
            CHAR_sendWatchEvent(CHAR_getWorkInt(index,CHAR_WORKOBJINDEX),
                                CHAR_ACTDAMAGE,opt,2,TRUE);
        }
        if( MAP_changeCharStatusFromMapDataAndTime(
            index,map[i], mode ? TRUE : FALSE ) )
            statuschange=TRUE;
    }
    if( damaged )       CHAR_sendStatusString(index,"M");
    if( statuschange ){
        CHAR_sendCToArroundCharacter(index);
        CHAR_sendStatusString(index,"P");
    }

}


/*----------------------------------------
 * 公及正奶伙卞曇月蟆卞裟壬木月〔仇仇匹公仇井日轉仁午｝xxx煙及
 * 儀毛  蜇允月〔
 * 婁醒
 *  index       int     曇欠丹午仄化中月平乓仿及奶件犯永弁旦
 *  flooor      int     白夫失ID
 *  fx          int     x甄  
 *  fy          int     y甄  
 * 忒曰襖
 *  卅仄
 ----------------------------------------*/
void MAP_preovered( int index )
{
    MAP_dealprepostevent( index, FALSE );
}

/*----------------------------------------
 * 公及正奶伙卞曇勻凶  卞裟壬木月〔
 * 婁醒
 *  index       int     曇欠丹午仄化中月平乓仿及奶件犯永弁旦
 * 忒曰襖
 *  卅仄
 ----------------------------------------*/
void MAP_postovered( int index )
{
    MAP_dealprepostevent( index, TRUE );
}


/*----------------------------------------
 * 平乓仿及璃曰及穴永皿毛允屯化霜月
 * 婁醒
 *  fd          int
 *  charaindex  int     平乓仿及奶件犯永弁旦
 * 忒曰襖
 *  岳      TRUE(1)
 *  撩      FALSE(0)
 ----------------------------------------*/
BOOL MAP_sendArroundCharNeedFD( int fd,int charaindex )
{
    char*   stringdata;
    int     x=CHAR_getInt(charaindex,CHAR_X);
    int     y=CHAR_getInt(charaindex,CHAR_Y);
    int     fl=CHAR_getInt(charaindex,CHAR_FLOOR);
    int     size=MAP_CHAR_DEFAULTSEESIZ;
    RECT    seekr,retr;
    seekr.x = x - (int)(size/2);
    seekr.y = y - (int)(size/2);
    seekr.width  = size;
    seekr.height = size;
#if 1
{
	int		tilesum, objsum, eventsum;
    stringdata = MAP_getChecksumFromRECT(fl,&seekr,&retr, &tilesum,&objsum,&eventsum);
    if( stringdata == NULL )
        return FALSE;

    lssproto_MC_send(fd,fl,
                    retr.x,              retr.y,
                    retr.x + retr.width, retr.y + retr.height,
                    tilesum,
                    objsum,
                    eventsum,
                    stringdata );
}
#else
    stringdata = MAP_getdataFromRECT(fl,&seekr,&retr);
    if( stringdata == NULL )
        return FALSE;

    lssproto_M_send(fd,fl,
                    retr.x,              retr.y,
                    retr.x + retr.width, retr.y + retr.height,
                    stringdata );
#endif
    return TRUE;
}


/*----------------------------------------
 * 穴永皿犯□正毛  月〔
 * 婁醒
 *  charaindex      int     平乓仿及奶件犯永弁旦
 * 忒曰襖
 *  岳      TRUE(1)
 *  撩      FALSE(0)
 ----------------------------------------*/
BOOL MAP_sendArroundChar(int charaindex)
{
    int fd;
    fd = getfdFromCharaIndex( charaindex );
    if( fd == -1 )return FALSE;

    return MAP_sendArroundCharNeedFD(fd, charaindex);
}

#ifndef _JA25GERMANTEXT__H_
#define _JA25GERMANTEXT__H_

enum {
  TCTL_MSG__RANGE_TO_TARGET,
  TCTL_MSG__ATTACH_TRANSMITTER_TO_LAPTOP,
  TACT_MSG__CANNOT_AFFORD_MERC,
  TACT_MSG__AIMMEMBER_FEE_TEXT,
  TACT_MSG__AIMMEMBER_ONE_TIME_FEE,
  TACT_MSG__FEE,
  TACT_MSG__SOMEONE_ELSE_IN_SECTOR,
  TCTL_MSG__RANGE_TO_TARGET_AND_GUN_RANGE,
  TCTL_MSG__DISPLAY_COVER,
  TCTL_MSG__LOS,
  TCTL_MSG__INVALID_DROPOFF_SECTOR,
  TCTL_MSG__PLAYER_LOST_SHOULD_RESTART,
  TCTL_MSG__JERRY_BREAKIN_LAPTOP_ANTENA,
  TCTL_MSG__END_GAME_POPUP_TXT_1,
  TCTL_MSG__END_GAME_POPUP_TXT_2,
  TCTL_MSG__IRON_MAN_CANT_SAVE_NOW,
  TCTL_MSG__CANNOT_SAVE_DURING_COMBAT,
  TCTL_MSG__CAMPAIGN_NAME_TOO_LARGE,
  TCTL_MSG__CAMPAIGN_DOESN_T_EXIST,
  TCTL_MSG__DEFAULT_CAMPAIGN_LABEL,
  TCTL_MSG__CAMPAIGN_LABEL,
  TCTL_MSG__NEW_CAMPAIGN_CONFIRM,
  TCTL_MSG__CANT_EDIT_DEFAULT,

};
extern STR16 zNewTacticalMessages[];

enum {
  IMM__IRON_MAN_MODE_WARNING_TEXT,
};
extern STR16 gzIronManModeWarningText[];

#endif

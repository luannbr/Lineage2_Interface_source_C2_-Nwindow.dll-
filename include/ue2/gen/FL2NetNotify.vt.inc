// FL2NetNotify: 352 slots from Engine.dll ??_7FL2NetNotify@@6B@ (tools/gen_engine_decls.py, do not edit)
virtual INT OnVersionCheck();                                                                  //   0 +0x000
virtual INT OnMoveToLocation(User*, FVector, FVector);                                         //   1 +0x004
virtual INT OnMoveToLocationInVehicle(FVehicle*, User*, FVector, FVector);                     //   2 +0x008
virtual INT OnMoveBackwardToLocation(User*, FVector, FVector);                                 //   3 +0x00c
virtual INT OnMoveToPawn(User*, User*, INT, FVector);                                          //   4 +0x010
virtual INT OnValidateLocation(User*, FVector, INT);                                           //   5 +0x014
virtual INT OnCheckGeo(FVector);                                                               //   6 +0x018
virtual INT OnValidateLocationInVehicle(FVehicle*, User*, FVector, INT);                       //   7 +0x01c
virtual INT OnFlyToLocation(User*, FVector, FVector);                                          //   8 +0x020
virtual INT OnEarthQuake(FVector, INT, INT, INT);                                              //   9 +0x024
virtual INT OnStartRotating(User*, INT, INT);                                                  //  10 +0x028
virtual INT OnFinishRotating(User*, INT);                                                      //  11 +0x02c
virtual INT OnNpcSay(L2ParamStack&);                                                           //  12 +0x030
virtual INT OnCharInfo(User*, FVector, INT, L2ParamStack&);                                    //  13 +0x034
virtual INT OnUserInfo(User*, FVector, INT, INT, INT, ANSICHAR);                               //  14 +0x038
virtual INT OnAttack(User*, User*, INT, INT, INT, INT, INT, FVector);                          //  16 +0x040
virtual INT OnAttack(User*, INT, INT, INT, INT, INT, FVector);                                 //  15 +0x03c
virtual INT OnAttacked(User*, User*);                                                          //  18 +0x048
virtual INT OnAttacked(User*, INT);                                                            //  17 +0x044
virtual INT OnAttackCanceled(User*);                                                           //  19 +0x04c
virtual INT OnDie(User*, L2ParamStack&);                                                       //  20 +0x050
virtual INT OnRide(User*, INT, INT, INT);                                                      //  21 +0x054
virtual INT OnReviveRequest(INT, INT);                                                         //  22 +0x058
virtual INT OnRevive(User*);                                                                   //  23 +0x05c
virtual INT OnAttackOutofRange(User*);                                                         //  24 +0x060
virtual INT OnAttackinCoolTime(User*);                                                         //  25 +0x064
virtual INT OnAttackDeadTarget(User*);                                                         //  26 +0x068
virtual INT OnLeaveWorld(User*);                                                               //  27 +0x06c
virtual INT OnAuthLoginSuccess();                                                              //  28 +0x070
virtual INT OnStartServerList(INT);                                                            //  29 +0x074
virtual INT OnAddServerList(INT, TCHAR*, INT, INT, INT, INT);                                  //  31 +0x07c
virtual INT OnAddServerList(L2ParamStack&);                                                    //  30 +0x078
virtual INT OnEndServerList(INT);                                                              //  32 +0x080
virtual INT OnAuthLoginFail(INT);                                                              //  33 +0x084
virtual INT OnBlockedAccountWithMsg(TCHAR*);                                                   //  34 +0x088
virtual INT OnBlockedAccount(INT);                                                             //  35 +0x08c
virtual INT OnCharacterCreateFail(INT);                                                        //  36 +0x090
virtual INT OnAuthServerSelectSuccess();                                                       //  37 +0x094
virtual INT OnAuthServerSelectFail(INT);                                                       //  38 +0x098
virtual INT OnSpawnItem(Item*, FVector, FRotator);                                             //  39 +0x09c
virtual INT OnDropItem(User*, Item*, FVector, FRotator);                                       //  40 +0x0a0
virtual INT OnDice(User*, INT, INT, FVector);                                                  //  41 +0x0a4
virtual INT OnEquipItemPlaySound(INT);                                                         //  42 +0x0a8
virtual INT OnGetItem(User*, Item*, FVector);                                                  //  43 +0x0ac
virtual INT OnEquipItem(User*, Item*, INT);                                                    //  44 +0x0b0
virtual INT OnUnequipItem(User*, Item*, INT);                                                  //  45 +0x0b4
virtual INT OnClassUpdate(User*, INT);                                                         //  46 +0x0b8
virtual INT OnLevelUpdate(User*, INT);                                                         //  47 +0x0bc
virtual INT OnExpUpdate(User*, INT);                                                           //  48 +0x0c0
virtual INT OnStrUpdate(User*, INT);                                                           //  49 +0x0c4
virtual INT OnDexUpdate(User*, INT);                                                           //  50 +0x0c8
virtual INT OnConUpdate(User*, INT);                                                           //  51 +0x0cc
virtual INT OnIntUpdate(User*, INT);                                                           //  52 +0x0d0
virtual INT OnWitUpdate(User*, INT);                                                           //  53 +0x0d4
virtual INT OnMenUpdate(User*, INT);                                                           //  54 +0x0d8
virtual INT OnHpUpdate(User*, INT);                                                            //  55 +0x0dc
virtual INT OnMaxHpUpdate(User*, INT);                                                         //  56 +0x0e0
virtual INT OnMpUpdate(User*, INT);                                                            //  57 +0x0e4
virtual INT OnMaxMpUpdate(User*, INT);                                                         //  58 +0x0e8
virtual INT OnSPUpdate(User*, INT);                                                            //  59 +0x0ec
virtual INT OnCarringWeightUpdate(User*, INT);                                                 //  60 +0x0f0
virtual INT OnCarryWeightUpdate(User*, INT);                                                   //  61 +0x0f4
virtual INT OnAttackRangeUpdate(User*, INT);                                                   //  62 +0x0f8
virtual INT OnPhysicalAttack(User*, INT);                                                      //  63 +0x0fc
virtual INT OnPhysicalAttackSpeed(User*, INT);                                                 //  64 +0x100
virtual INT OnPhysicalDefense(User*, INT);                                                     //  65 +0x104
virtual INT OnPhysicalAvoid(User*, INT);                                                       //  66 +0x108
virtual INT OnHitRate(User*, INT);                                                             //  67 +0x10c
virtual INT OnCriticalRate(User*, INT);                                                        //  68 +0x110
virtual INT OnMagicalAttack(User*, INT);                                                       //  69 +0x114
virtual INT OnMagicCastingSpeed(User*, INT);                                                   //  70 +0x118
virtual INT OnMagicDefense(User*, INT);                                                        //  71 +0x11c
virtual INT OnGuiltyUpdate(User*, INT);                                                        //  72 +0x120
virtual INT OnCriminalRateUpdate(User*, INT);                                                  //  73 +0x124
virtual INT OnNpcHtmlMessage(User*, TCHAR*);                                                   //  74 +0x128
virtual INT OnTutorialShowHtml(TCHAR*);                                                        //  75 +0x12c
virtual INT OnTutorialShowQuestionMark(INT);                                                   //  76 +0x130
virtual INT OnTutorialEnableClientEvent(INT);                                                  //  77 +0x134
virtual INT OnTutorialCloseHtml();                                                             //  78 +0x138
virtual INT CheckTutorialClientEvent(INT);                                                     //  79 +0x13c
virtual INT OnShowRadar(INT, INT, INT, INT, INT);                                              //  80 +0x140
virtual INT OnDeleteRadar(INT, INT, INT);                                                      //  81 +0x144
virtual INT OnStartSellList(INT, INT);                                                         //  82 +0x148
virtual INT OnAddSellList(ItemInfo&);                                                          //  83 +0x14c
virtual INT OnStartBuyList(INT, INT);                                                          //  84 +0x150
virtual INT OnAddBuyList(ItemInfo&);                                                           //  85 +0x154
virtual INT OnStartManorSellList(INT, INT);                                                    //  86 +0x158
virtual INT OnAddManorSellList(ItemInfo&);                                                     //  87 +0x15c
virtual INT OnStartManorBuyList(INT, INT);                                                     //  88 +0x160
virtual INT OnAddManorBuyList(ItemInfo&);                                                      //  89 +0x164
virtual INT OnStartDepositList(INT);                                                           //  90 +0x168
virtual INT OnAddDepositList(ItemInfo&);                                                       //  91 +0x16c
virtual INT OnStartWithdrawList(INT);                                                          //  92 +0x170
virtual INT OnAddWithdrawList(ItemInfo&);                                                      //  93 +0x174
virtual INT OnDeleteUser(User*, INT, INT);                                                     //  94 +0x178
virtual INT OnDeleteItem(Item*);                                                               //  95 +0x17c
virtual INT OnGMHide(INT);                                                                     //  96 +0x180
virtual INT OnServerObjectInfo(User*, FVector, INT, INT, INT);                                 //  97 +0x184
virtual INT OnNpcInfo(User*, FVector, INT, L2ParamStack&);                                     //  98 +0x188
virtual INT OnActionFail();                                                                    //  99 +0x18c
virtual INT OnTeleportToLocation(User*, FVector);                                              // 100 +0x190
virtual INT OnTargetSelected(User*, INT, User*, Item*, FServerStaticObject*, FVector);         // 101 +0x194
virtual INT OnMyTargetSelected(User*, INT, Item*, FServerStaticObject*, INT);                  // 102 +0x198
virtual INT OnTargetUnselected(User*, FVector);                                                // 103 +0x19c
virtual INT OnAutoAttackStart(User*);                                                          // 104 +0x1a0
virtual INT OnAutoAttackStop(User*);                                                           // 105 +0x1a4
virtual INT OnSocialAction(User*, INT);                                                        // 106 +0x1a8
virtual INT OnChangeMoveType(User*, INT, INT);                                                 // 107 +0x1ac
virtual INT OnChangeWaitType(User*, INT, FVector);                                             // 108 +0x1b0
virtual INT OnStopMove(User*, FVector, INT);                                                   // 109 +0x1b4
virtual INT OnStopMoveInVehicle(FVehicle*, User*, FVector, INT);                               // 110 +0x1b8
virtual INT OnStopMoveWithLocation(User*, FVector);                                            // 111 +0x1bc
virtual INT OnDoorInfo(L2ParamStack&, AActor**);                                               // 112 +0x1c0
virtual INT OnDoorStatusUpdate(L2ParamStack&, AActor*);                                        // 113 +0x1c4
virtual INT OnVehicleInfo(FVehicle*, FVector, INT);                                            // 114 +0x1c8
virtual INT OnDeleteVehicle(FVehicle*);                                                        // 115 +0x1cc
virtual INT OnVehicleDeparture(FVehicle*, INT, INT, FVector);                                  // 116 +0x1d0
virtual INT OnVehicleCheckLocation(FVehicle*, FVector, INT);                                   // 117 +0x1d4
virtual INT OnGetOnVehicle(User*, FVehicle*, FVector);                                         // 118 +0x1d8
virtual INT OnGetOffVehicle(User*, FVehicle*, FVector);                                        // 119 +0x1dc
virtual INT OnSpecialCamera(User*, FLOAT, INT, INT, FLOAT, FLOAT);                             // 120 +0x1e0
virtual INT OnNormalCamera();                                                                  // 121 +0x1e4
virtual INT OnEventTrigger(INT, INT);                                                          // 122 +0x1e8
virtual INT OnSkillRemainSec(User*, User*, INT, INT, INT, INT, FLOAT);                         // 123 +0x1ec
virtual void OnAcceptLogOn();                                                                  // 124 +0x1f0
virtual void OnAcceptLogOut();                                                                 // 125 +0x1f4
virtual INT CheckLoginType(INT);                                                               // 126 +0x1f8
virtual void OnStartNotice(L2ParamStack&);                                                     // 127 +0x1fc
virtual void OnSCCheckStart(INT, INT);                                                         // 128 +0x200
virtual void OnReceiveCharacterInfo();                                                         // 129 +0x204
virtual void AddCharacterInfo(User);                                                           // 130 +0x208
virtual void OnAcceptGameStart(INT);                                                           // 131 +0x20c
virtual void OnAcceptCharacterDelete(INT);                                                     // 132 +0x210
virtual void OnAcceptNewCharacter(INT);                                                        // 133 +0x214
virtual void OnAcceptCharacterCreate(INT);                                                     // 134 +0x218
virtual void OnReceiveItemList(INT);                                                           // 135 +0x21c
virtual void OnReceiveUpdateItemList(INT, ItemInfo&);                                          // 136 +0x220
virtual void OnReceiveTradeStart(INT);                                                         // 137 +0x224
virtual void OnReceiveAddTradeInventoryItem(ItemInfo&);                                        // 138 +0x228
virtual void OnReceiveAddTradeOwnItem(ItemInfo&);                                              // 139 +0x22c
virtual void OnReceiveAddTradeOtherItem(ItemInfo&);                                            // 140 +0x230
virtual void OnReceiveTradeUpdate(INT, ItemInfo&);                                             // 141 +0x234
virtual void OnReceiveTradeDoneMsg(INT);                                                       // 142 +0x238
virtual void OnReceiveCharacterSelectedPacket(INT, INT, INT, INT, void*, INT, INT, INT, INT);  // 143 +0x23c
virtual void AddRecommandedData(RecommandedDataStr);                                           // 144 +0x240
virtual void EndRecommandedData();                                                             // 145 +0x244
virtual void AddInventoryItem(ItemInfo&);                                                      // 146 +0x248
virtual void OnUpdatePlayerInfo(L2ParamStack&);                                                // 147 +0x24c
virtual void OnServerClose(INT);                                                               // 148 +0x250
virtual void OnReceiveHtmlMessage(L2ParamStack&);                                              // 149 +0x254
virtual void OnReceivePledgeExtendedInfo(PledgeInfo&);                                         // 150 +0x258
virtual void OnReceivePledgeInfo(PledgeInfo&);                                                 // 151 +0x25c
virtual void OnReceivePledgeInfoUpdate(PledgeInfo&);                                           // 152 +0x260
virtual void OnReceivePledgeMemberList();                                                      // 153 +0x264
virtual void OnAddPledgeMember(L2ParamStack&);                                                 // 154 +0x268
virtual void OnUpdatePledgeMember(L2ParamStack&);                                              // 155 +0x26c
virtual void OnDeletePledgeMember(TCHAR*);                                                     // 156 +0x270
virtual void OnDeleteAllPledgeMember();                                                        // 157 +0x274
virtual void OnReceiveCreatePledge(L2ParamStack&);                                             // 158 +0x278
virtual void OnImportCrestFile(INT, INT, INT);                                                 // 159 +0x27c
virtual void OnMakeDDSCrestFile(INT, INT, BYTE*, INT, INT);                                    // 160 +0x280
virtual void OnReceiveAskJoinPledge(L2ParamStack&);                                            // 161 +0x284
virtual void OnReceiveOustPledgeMember(L2ParamStack&);                                         // 162 +0x288
virtual void OnReceiveDismissPledge(L2ParamStack&);                                            // 163 +0x28c
virtual void OnReceiveJoinPledge(L2ParamStack&);                                               // 164 +0x290
virtual void OnReceiveWidthdrawalPledge(L2ParamStack&);                                        // 165 +0x294
virtual void OnReceiveOustPledgeMessage(L2ParamStack&);                                        // 166 +0x298
virtual void OnReceiveDismissPledgeMessage(L2ParamStack&);                                     // 167 +0x29c
virtual void OnReceiveStartPledgeWar(L2ParamStack&);                                           // 168 +0x2a0
virtual void OnReceiveReplyStartPledgeWar(L2ParamStack&);                                      // 169 +0x2a4
virtual void OnReceiveStopPledgeWar(L2ParamStack&);                                            // 170 +0x2a8
virtual void OnReceiveReplyStopPledgeWar(L2ParamStack&);                                       // 171 +0x2ac
virtual void OnReceiveSurrenderPledgeWar(L2ParamStack&);                                       // 172 +0x2b0
virtual void OnReceiveReplySurrenderPledgeWar(L2ParamStack&);                                  // 173 +0x2b4
virtual void OnReceiveSurrenderPersonally(L2ParamStack&);                                      // 174 +0x2b8
virtual void OnReceiveAskJoinAlly(L2ParamStack&);                                              // 175 +0x2bc
virtual void OnReceiveOustAlly(L2ParamStack&);                                                 // 176 +0x2c0
virtual void OnReceiveDismissAlly(L2ParamStack&);                                              // 177 +0x2c4
virtual void OnReceiveJoinAlly(L2ParamStack&);                                                 // 178 +0x2c8
virtual void OnReceiveWithdrawAlly(L2ParamStack&);                                             // 179 +0x2cc
virtual void OnReceiveFriendAddRequestResult(L2ParamStack&);                                   // 180 +0x2d0
virtual void OnReceiveFriendAddRequest(L2ParamStack&);                                         // 181 +0x2d4
virtual void OnReceiveFriendAdd(L2ParamStack&);                                                // 182 +0x2d8
virtual void OnReceiveFriendRemove(L2ParamStack&);                                             // 183 +0x2dc
virtual void OnReceiveFriendList(L2ParamStack&);                                               // 184 +0x2e0
virtual void OnReceiveFriendStatus(L2ParamStack&);                                             // 185 +0x2e4
virtual void OnReceiveBlockList(L2ParamStack&);                                                // 186 +0x2e8
virtual void OnReceiveAskJoinParty(L2ParamStack&);                                             // 187 +0x2ec
virtual void OnReceiveJoinParty(L2ParamStack&);                                                // 188 +0x2f0
virtual void OnReceiveWithdrawalParty(L2ParamStack&);                                          // 189 +0x2f4
virtual void OnReceiveOustPartyMember(L2ParamStack&);                                          // 190 +0x2f8
virtual void OnReceiveOustPartyMessage(L2ParamStack&);                                         // 191 +0x2fc
virtual void OnReceiveDismissParty(L2ParamStack&);                                             // 192 +0x300
virtual void OnRecevieDismissPartyMessage(L2ParamStack&);                                      // 193 +0x304
virtual void OnReceiveWareHouseDonePacket(L2ParamStack&);                                      // 194 +0x308
virtual void OnReceiveShortCutList();                                                          // 195 +0x30c
virtual void AddShortCutItem(L2ParamStack&);                                                   // 196 +0x310
virtual void OnUpdateShortCutItem(INT);                                                        // 197 +0x314
virtual void OnUpdatePetShortCutItem(INT);                                                     // 198 +0x318
virtual void OnReceiveShortCutDelete(INT);                                                     // 199 +0x31c
virtual void OnReceiveMagicSkillListPacket(L2ParamStack&);                                     // 200 +0x320
virtual void OnSay2(L2ParamStack&);                                                            // 201 +0x324
virtual void OnReceiveVersionCheck(L2ParamStack&);                                             // 202 +0x328
virtual void OnReceiveAbnormalStatus();                                                        // 203 +0x32c
virtual void AddAbnormalStatus(L2ParamStack&);                                                 // 204 +0x330
virtual void OnReceiveQuestList();                                                             // 205 +0x334
virtual void AddQuestID(INT, INT);                                                             // 206 +0x338
virtual void AddQuestItem(ItemInfo&);                                                          // 207 +0x33c
virtual void OnUpdateEquipItem(L2ParamStack&);                                                 // 208 +0x340
virtual void ReceivePartyMatchList();                                                          // 209 +0x344
virtual void AddPartyMatchMember(L2ParamStack&);                                               // 210 +0x348
virtual void OnReceivePartyMatchDetail(L2ParamStack&);                                         // 211 +0x34c
virtual void OnPartyAddMemberPacket(L2ParamStack&);                                            // 212 +0x350
virtual void OnPartyUpdateMemberPacket(L2ParamStack&);                                         // 213 +0x354
virtual void OnPartyDeleteMemberPacket(L2ParamStack&);                                         // 214 +0x358
virtual void OnPartyDeleteAllPacket(L2ParamStack&);                                            // 215 +0x35c
virtual void OnShowMagicWnd();                                                                 // 216 +0x360
virtual void OnShowSkillWnd();                                                                 // 217 +0x364
virtual void OnSkillListPacket(L2ParamStack&);                                                 // 218 +0x368
virtual void OnMagicListPacket(L2ParamStack&);                                                 // 219 +0x36c
virtual void OnReceiveTradeDemand(L2ParamStack&);                                              // 220 +0x370
virtual void OnRestartResponse(L2ParamStack&);                                                 // 221 +0x374
virtual void OnAddSystemMessageParam(L2ParamStack&);                                           // 222 +0x378
virtual void OnEndSystemMessageParam(L2ParamStack&);                                           // 223 +0x37c
virtual void OnSetupGaugePacket(L2ParamStack&);                                                // 224 +0x380
virtual void OnShowboardPacket(L2ParamStack&);                                                 // 225 +0x384
virtual void OnSelectItemToEnchant();                                                          // 226 +0x388
virtual void OnReceiveEnchantResult(INT);                                                      // 227 +0x38c
virtual void OnReceiveAction(INT);                                                             // 228 +0x390
virtual void OnShowTrainListWnd();                                                             // 229 +0x394
virtual void OnShowTrainInfoWnd(L2ParamStack&);                                                // 230 +0x398
virtual void AddAcquireSkillInfo(L2ParamStack&);                                               // 231 +0x39c
virtual void AddAcquireSkillExtendedInfo(L2ParamStack&);                                       // 232 +0x3a0
virtual void OnHideTrainWnd();                                                                 // 233 +0x3a4
virtual void OnReceiveRecipeBookItemList(INT);                                                 // 234 +0x3a8
virtual void AddRecipeBookItem(INT);                                                           // 235 +0x3ac
virtual void ReceiveRecipeItemMakeInfo(INT, INT, INT, INT);                                    // 236 +0x3b0
virtual void ReceiveRecipeShopManageList(INT, INT);                                            // 237 +0x3b4
virtual void AddRecipeShopManageItem(INT, INT);                                                // 238 +0x3b8
virtual void AddRecipeShopRegisterdItem(INT, INT, INT);                                        // 239 +0x3bc
virtual void ReceiveRecipeShopSellList(INT, INT, INT, INT);                                    // 240 +0x3c0
virtual void AddRecipeShopSellItem(INT, INT, INT);                                             // 241 +0x3c4
virtual void ReceiveRecipeShopItemInfo(INT, INT, INT, INT, INT);                               // 242 +0x3c8
virtual void RequestRecipeBookOpen();                                                          // 243 +0x3cc
virtual void RequestRecipeItemDelete(INT);                                                     // 244 +0x3d0
virtual void RequestRecipeItemMakeInfo(INT);                                                   // 245 +0x3d4
virtual void RequestRecipeItemMakeSelf(INT);                                                   // 246 +0x3d8
virtual void RequestRecipeShopManageList();                                                    // 247 +0x3dc
virtual void RequestRecipeShopMessageSet(TCHAR*);                                              // 248 +0x3e0
virtual void RequestRecipeShopListSet(L2ParamStack&);                                          // 249 +0x3e4
virtual void RequestRecipeShopManageQuit();                                                    // 250 +0x3e8
virtual void RequestRecipeShopManageCancel();                                                  // 251 +0x3ec
virtual void RequestRecipeShopSellList(INT);                                                   // 252 +0x3f0
virtual void RequestRecipeShopMakeInfo(INT, INT);                                              // 253 +0x3f4
virtual void RequestRecipeShopMakeDo(INT, INT);                                                // 254 +0x3f8
virtual void ReceiveMacroListInfo(ANSICHAR, ANSICHAR, MacroInfo, ANSICHAR, INT);               // 255 +0x3fc
virtual void ReceiveHennaEquipList(INT, INT);                                                  // 256 +0x400
virtual void AddHennaEquipInfo(HennaInfo);                                                     // 257 +0x404
virtual void ReceiveHennaItemInfo(HennaInfo, INT);                                             // 258 +0x408
virtual void ReceiveHennaInfo(HennaInfo, INT);                                                 // 259 +0x40c
virtual void AddHennaInfo(INT, INT);                                                           // 260 +0x410
virtual void ReceiveHennaUnequipList(INT, INT);                                                // 261 +0x414
virtual void AddHennaUnequipInfo(HennaInfo);                                                   // 262 +0x418
virtual void ReceiveHennaUnequipItemInfo(HennaInfo, INT);                                      // 263 +0x41c
virtual void RequestHennaItemInfo(INT);                                                        // 264 +0x420
virtual void RequestHennaEquip(INT);                                                           // 265 +0x424
virtual void RequestHennaUnequipInfo(INT);                                                     // 266 +0x428
virtual void RequestHennaUnequip(INT);                                                         // 267 +0x42c
virtual void RequestHennaItemList();                                                           // 268 +0x430
virtual void RequestHennaUnequipList();                                                        // 269 +0x434
virtual void GMAddHennaInfo(INT, INT);                                                         // 270 +0x438
virtual void GMReceiveHennaInfo(HennaInfo, INT);                                               // 271 +0x43c
virtual void EnablePartyChat(bool);                                                            // 272 +0x440
virtual void EnablePledgeChat(bool);                                                           // 273 +0x444
virtual void EnableAllyChat(bool);                                                             // 274 +0x448
virtual void OnShowPetStatusWnd(INT);                                                          // 275 +0x44c
virtual void OnShowPetPartyWnd(INT);                                                           // 276 +0x450
virtual void OnHidePetWnd(INT);                                                                // 277 +0x454
virtual void OnShowPetNameBtn(INT);                                                            // 278 +0x458
virtual void OnReceivePetStatusInfo(INT, L2ParamStack&);                                       // 279 +0x45c
virtual void OnReceivePetItemList();                                                           // 280 +0x460
virtual void AddPetInventoryItem(ItemInfo&);                                                   // 281 +0x464
virtual void UpdatePetInventoryItem(INT, ItemInfo&);                                           // 282 +0x468
virtual void OnGMViewCharacterInfo(User&);                                                     // 283 +0x46c
virtual void OnGMViewReceivePledgeInfo(PledgeInfo&);                                           // 284 +0x470
virtual void OnGMViewReceivePledgeMemberList();                                                // 285 +0x474
virtual void OnGMViewAddPledgeMember(L2ParamStack&);                                           // 286 +0x478
virtual void OnGMViewShowSkillWnd();                                                           // 287 +0x47c
virtual void OnGMViewSkillListPacket(L2ParamStack&);                                           // 288 +0x480
virtual void OnGMViewShowMagicWnd();                                                           // 289 +0x484
virtual void OnGMViewMagicListPacket(L2ParamStack&);                                           // 290 +0x488
virtual void OnGMViewReceiveQuestList();                                                       // 291 +0x48c
virtual void GMViewAddQuestID(INT, INT);                                                       // 292 +0x490
virtual void GMViewAddQuestItem(ItemInfo&);                                                    // 293 +0x494
virtual void OnGMViewReceiveItemList(INT);                                                     // 294 +0x498
virtual void GMViewAddInventoryItem(ItemInfo&);                                                // 295 +0x49c
virtual void OnGMViewStartWithdrawList(INT);                                                   // 296 +0x4a0
virtual void OnGMViewAddWithdrawList(ItemInfo&);                                               // 297 +0x4a4
virtual void AddPackageToList(L2ParamStack&);                                                  // 298 +0x4a8
virtual void AddSendableItemList(INT, INT, INT, ItemInfo&);                                    // 299 +0x4ac
virtual void OnShowMinimap(INT);                                                               // 300 +0x4b0
virtual void OnShowCalc(INT);                                                                  // 301 +0x4b4
virtual void OnStartPrivateStoreManageList();                                                  // 302 +0x4b8
virtual void OnStartPrivateStoreList();                                                        // 303 +0x4bc
virtual void OnPrivateStoreManageList(ItemInfo&, ItemInfo&, INT, INT);                         // 304 +0x4c0
virtual void OnPrivateStoreList(ItemInfo&, INT, INT);                                          // 305 +0x4c4
virtual void OnPrivateStoreMsg(TCHAR*, INT);                                                   // 306 +0x4c8
virtual void OnStartPrivateStoreBuyManageList();                                               // 307 +0x4cc
virtual void OnStartPrivateStoreBuyList();                                                     // 308 +0x4d0
virtual void OnPrivateStoreBuyManageList(ItemInfo&, ItemInfo&, INT, INT);                      // 309 +0x4d4
virtual void OnPrivateStoreBuyList(ItemInfo&, INT, INT);                                       // 310 +0x4d8
virtual void OnPrivateStoreBuyMsg(TCHAR*, INT);                                                // 311 +0x4dc
virtual void OnPlaySound(L2ParamStack&);                                                       // 312 +0x4e0
virtual void OnInterfacePlaySound(TCHAR*);                                                     // 313 +0x4e4
virtual INT OnStaticObjectInfo(L2ParamStack&, AActor**);                                       // 314 +0x4e8
virtual void OnDeleteStaticObject(FServerStaticObject*);                                       // 315 +0x4ec
virtual INT OnMoveToActor(User*, AActor*, INT, FVector);                                       // 316 +0x4f0
virtual void OnReceiveMagicSkillUse(User*, User*, L2ParamStack&);                              // 318 +0x4f8
virtual void OnReceiveMagicSkillUse(User*, INT, L2ParamStack&);                                // 317 +0x4f4
virtual void OnReceiveMagicSkillCanceled(User*);                                               // 319 +0x4fc
virtual INT OnReceiveMagicSkillLaunched(User*, User*, INT, INT, INT);                          // 321 +0x504
virtual INT OnReceiveMagicSkillLaunched(User*, INT, INT, INT, INT);                            // 320 +0x500
virtual INT OnAssociatedAttack(User*, User*, INT, INT, INT, INT, INT, FVector);                // 322 +0x508
virtual void OnVehicleStartPacket(L2ParamStack&);                                              // 323 +0x50c
virtual void OnRequestTimeCheckPacket(L2ParamStack&);                                          // 324 +0x510
virtual void OnReceiveServerTime(INT);                                                         // 325 +0x514
virtual void OnSkillCoolTimePacket(L2ParamStack&);                                             // 326 +0x518
virtual void OnStartAllianceWar(L2ParamStack&);                                                // 327 +0x51c
virtual void OnStopAllianceWar(L2ParamStack&);                                                 // 328 +0x520
virtual void OnReceiveSiegeInfo(L2ParamStack&);                                                // 329 +0x524
virtual void OnReceiveSiegeAttackerList(L2ParamStack&);                                        // 330 +0x528
virtual void OnReceiveSiegeDefenderList(L2ParamStack&);                                        // 331 +0x52c
virtual void OnAddSelectableTime(INT);                                                         // 332 +0x530
virtual void OnEndSiegeAttackerlist();                                                         // 333 +0x534
virtual void OnEndSiegeDefenderlist();                                                         // 334 +0x538
virtual void ClearAttackerList();                                                              // 335 +0x53c
virtual void ClearDefenderList();                                                              // 336 +0x540
virtual void OnSetVIPShopID(INT);                                                              // 337 +0x544
virtual void OnAddVIPItemList(INT, INT, INT, INT, INT);                                        // 338 +0x548
virtual void OnAddVIPNeedItemList(INT, INT, INT);                                              // 339 +0x54c
virtual void OnVIPItemListEnd();                                                               // 340 +0x550
virtual void OnSetSummonRemainTime(INT, INT);                                                  // 341 +0x554
virtual void SetServerPing(INT, DWORD);                                                        // 342 +0x558
virtual void OnEndItemList();                                                                  // 343 +0x55c
virtual void OnSnoop(L2ParamStack&);                                                           // 344 +0x560
virtual void OnShowTownMapPacket(L2ParamStack&);                                               // 345 +0x564
virtual void OnObserverStartPacket(L2ParamStack&);                                             // 346 +0x568
virtual void OnObserverEndPacket(L2ParamStack&);                                               // 347 +0x56c
virtual void OnChairSitPacket(L2ParamStack&);                                                  // 348 +0x570
virtual void OnPledgePower(INT, INT, BYTE*);                                                   // 349 +0x574
virtual void OnShowXMasSeal(INT);                                                              // 350 +0x578
virtual void OnGameGuardQuery(DWORD, DWORD, DWORD, DWORD);                                     // 351 +0x57c

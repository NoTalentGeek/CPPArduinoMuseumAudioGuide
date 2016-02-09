/*Arduiono program for museum guide.*/
#include                    "ButtonObject.h"
#include                    "QueueList.h"
#include                    "RFIDReaderObject.h"
#include                    "VS1053Object.h"

#define                     PIN_REPEAT                                          22
#define                     PIN_VOLUME_DOWN                                     23                                  /*Device volume down    button      pin.                                */
#define                     PIN_VOLUME_UP                                       24                                  /*Device volume up      button      pin.                                */

        boolean             initialBoolean                                      = true;
        boolean             justDonePlayingAudioFileBoolean                     = false;                            /*Whether or not the device just finished playing an audio file.        */
        boolean             stillCapturingBoolean                               = false;
        char                audioFileAnyCharArray               [10]            = "92.mp3";                         /*Reference to audio file that plays any.mp3.                           */
        char                audioFileExhibitionCharArray        [10]            = "93.mp3";                         /*Reference to audio file that plays exhibition.mp3.                    */
        char                audioFileExplanationCharArray       [10]            = "94.mp3";                         /*Reference to audio file that plays explanation.mp3.                   */
        char                audioFileIndexCharArray             [10]            = "1.mp3";                          /*A temporary char array that will contain a converted string.          */
        char                audioFileMuseumCharArray            [10]            = "95.mp3";                         /*Reference to audio file that plays museum.mp3.                        */
        char                audioFileOrCharArray                [10]            = "96.mp3";                         /*Reference to audio file that plays or.mp3.                            */
        char                audioFilePleaseVisitAndTapCharArray [10]            = "97.mp3";                         /*Reference to audio file that plays pleaseTapAndVisit.mp3.             */
        char                audioFileSampleCharArray            [10]            = "98.mp3";                         /*Reference to audio file that plays sample.mp3.                        */
        char                audioFileWelcomeToCharArray         [10]            = "99.mp3";                         /*Reference to audio file that plays welcomeTo.mp3.                     */
        char                exhibitionCurrentIndexCharArray     [10]            ;
        char                exhibitionExplanationIndexCharArray [10]            ;
        char                exhibitionTargetIndex1CharArray     [10]            ;
        char                exhibitionTargetIndex2CharArray     [10]            ;
        char                exhibitionTargetIndex3CharArray     [10]            ;
        char*               repeatInstructionCharPointerArray   [14]            ;
        int                 audioFileIndexInt                                   = 0;                                /*PROTOTYPE: Current audio index.                                       */
        int                 audioFileIndexMaxInt                                = 8;                                /*The latest exhibition index in this device.                           */
        int                 audioFileIndexMinInt                                = 1;                                /*The least exhibition index in this device.                            */
        int                 exhibitionCurrentIndexInt                           ;
        int                 exhibitionExplanationIndexInt                       ;
        int                 exhibitionReceiveIndexInt                           = -1;                               /*The receive index from RFID.                                          */
        int                 exhibitionReceiveIndexSaveInt                       = -1;                               /*The receive index from RFID (non null).                               */
        int                 exhibitionTargetFillInCounterInt                    = 0;
        int                 exhibitionTargetIndex1Int                           ;
        int                 exhibitionTargetIndex2Int                           ;
        int                 exhibitionTargetIndex3Int                           ;
        int                 playerIndexInt                                      = 1;                                /*The player index of this device.                                      */
        ButtonObject        repeatButtonObject                                  = ButtonObject(PIN_REPEAT);
        ButtonObject        volumeDownButtonObject                              = ButtonObject(PIN_VOLUME_DOWN);    /*Device volume down    button   object.                                */
        ButtonObject        volumeUpButtonObject                                = ButtonObject(PIN_VOLUME_UP  );    /*Device volume up      button   object.                                */
        QueueList<char*>    repeatInstructionCharPointerQueueList               ;
        RFIDReaderObject    rfidReaderObject                                    = RFIDReaderObject();
        String              exhibitionCurrentIndexString                        ;
        String              exhibitionExplanationIndexString                    ;
        String              exhibitionTargetIndex1String                        ;
        String              exhibitionTargetIndex2String                        ;
        String              exhibitionTargetIndex3String                        ;
        VS1053Object        vs1053Object                                        = VS1053Object();

void        setup                                                               (){

                Serial                                                          .begin(9600);                       /*Initiate Sereial connection.                                          */
                rfidReaderObject                                                .SetupVoid();                       /*Setup the RFID component.                                             */
                vs1053Object                                                    .SetupVoid();                       /*Setup the VS1053 audio player.                                        */
                //EstablishProcessingConnection                                 ();                                 /*Establish handshake connection for software controller.               */

                Serial.println("HANDSHAKE");

                /*Play the file and welcome the visitor for the first time he/she enter the museum.*/
                vs1053Object.GetAdafruitVS1053FilePlayer().playFullFile         (audioFileWelcomeToCharArray);
                vs1053Object.GetAdafruitVS1053FilePlayer().playFullFile         (audioFileSampleCharArray);
                vs1053Object.GetAdafruitVS1053FilePlayer().playFullFile         (audioFileMuseumCharArray);
                vs1053Object.GetAdafruitVS1053FilePlayer().playFullFile         (audioFilePleaseVisitAndTapCharArray);
                vs1053Object.GetAdafruitVS1053FilePlayer().playFullFile         (audioFileAnyCharArray);
                vs1053Object.GetAdafruitVS1053FilePlayer().playFullFile         (audioFileExhibitionCharArray);

}
void        loop                                                                (){

                exhibitionReceiveIndexInt                                       = rfidReaderObject.LoopVoid(vs1053Object.GetPlayingMusicBool());    /*Always request exhibition index value from the RFID reader.                                                           */
                if(exhibitionReceiveIndexInt != -1)                             { exhibitionReceiveIndexSaveInt = exhibitionReceiveIndexInt; }      /*The the exhibition index value received from the RFID is -1 then do not process the value.                            */
                MoveExhibitionVoid                                              ();                                                                 /*If the exhibition index value received is legit (not -1) then process with moving the player into new exhibition.     */

                if( repeatButtonObject.ButtonPressedBoolean()       == true &&
                    stillCapturingBoolean == false)                             { RepeatInstructionVoid(initialBoolean);    }
                if( volumeDownButtonObject.ButtonPressedBoolean()   == true &&
                    stillCapturingBoolean == false)                             { vs1053Object.VolumeDownVoid();            }
                if( volumeUpButtonObject.ButtonPressedBoolean()     == true &&
                    stillCapturingBoolean == false)                             { vs1053Object.VolumeUpVoid();              }
                /*
                volumeDownButtonObject                                          .ButtonVolumeDownLoopVoid   (vs1053Object);
                volumeUpButtonObject                                            .ButtonVolumeUpLoopVoid     (vs1053Object);
                */

                /*Send instruction to software controller that this device has finished playing an audio file.*/
                if( justDonePlayingAudioFileBoolean    == true ){
                    Serial                                                      .println("O"); /*DONE_PLAYING_AUDIO_FILE*/
                    justDonePlayingAudioFileBoolean                             =  false;
                }

                /*Convert Serial commmand into playing some specific audio file.*/
                if(Serial.available() > 0){
                    String   inputString                                                = Serial.readString();

                    if      (
                                (inputString                                            == "Q"      ) && /*PLAY_EXHIBITION*/
                                (justDonePlayingAudioFileBoolean                        == false    ) &&
                                (stillCapturingBoolean                                  == true     )
                            ){

                                vs1053Object.GetAdafruitVS1053FilePlayer()              .playFullFile   (audioFileExhibitionCharArray);
                                repeatInstructionCharPointerQueueList                   .push           (audioFileExhibitionCharArray);
                                justDonePlayingAudioFileBoolean                         = true;

                    }

                    if      (
                                (inputString                                            == "W"      ) && /*PLAY_EXHIBITION_VISITED*/
                                (justDonePlayingAudioFileBoolean                        == false    ) &&
                                (stillCapturingBoolean                                  == true     )
                            ){

                                vs1053Object.GetAdafruitVS1053FilePlayer()              .playFullFile   (AudioIndexCharArray(exhibitionReceiveIndexSaveInt + 1));

                                exhibitionCurrentIndexInt                               = exhibitionReceiveIndexSaveInt + 1;
                                exhibitionCurrentIndexString                            = String        (exhibitionCurrentIndexInt) + ".mp3";
                                exhibitionCurrentIndexString                            .toCharArray    (exhibitionCurrentIndexCharArray, 10);
                                repeatInstructionCharPointerQueueList                   .push           (exhibitionCurrentIndexCharArray);

                                justDonePlayingAudioFileBoolean                         = true;
                                exhibitionReceiveIndexSaveInt                           = -1;

                    }

                    if      (
                                (inputString                                            == "E"      ) && /*PLAY_EXPLANATION*/
                                (justDonePlayingAudioFileBoolean                        == false    ) &&
                                (stillCapturingBoolean                                  == true     )
                            ){

                                vs1053Object.GetAdafruitVS1053FilePlayer()              .playFullFile   (audioFileExplanationCharArray);
                                repeatInstructionCharPointerQueueList                   .push           (audioFileExplanationCharArray);
                                justDonePlayingAudioFileBoolean                         = true;

                    }

                    if      (
                                (inputString                                            == "R"      ) && /*PLAY_OR*/
                                (justDonePlayingAudioFileBoolean                        == false    ) &&
                                (stillCapturingBoolean                                  == true     )
                            ){

                                vs1053Object.GetAdafruitVS1053FilePlayer()              .playFullFile(audioFileOrCharArray);
                                repeatInstructionCharPointerQueueList                   .push(audioFileOrCharArray);
                                justDonePlayingAudioFileBoolean                         = true;

                    }

                    if      (
                                (inputString                                            == "T"      ) && /*PLAY_PLEASE_VISIT_AND_TAP*/
                                (justDonePlayingAudioFileBoolean                        == false    ) &&
                                (stillCapturingBoolean                                  == true     )
                            ){

                                vs1053Object.GetAdafruitVS1053FilePlayer()              .playFullFile(audioFilePleaseVisitAndTapCharArray);
                                repeatInstructionCharPointerQueueList                   .push(audioFilePleaseVisitAndTapCharArray);
                                justDonePlayingAudioFileBoolean                         = true;

                    }

                    if      (
                                (inputString                                            == "Y"      ) && /*PLAY_WELCOME*/
                                (justDonePlayingAudioFileBoolean                        == false    ) &&
                                (stillCapturingBoolean                                  == true     )
                            ){

                                vs1053Object.GetAdafruitVS1053FilePlayer()              .playFullFile(audioFileWelcomeToCharArray);
                                repeatInstructionCharPointerQueueList                   .push(audioFileWelcomeToCharArray);
                                justDonePlayingAudioFileBoolean                         = true;

                    }

                    if      (inputString == "U"){ /*CAPTURE_DONE*/
                                stillCapturingBoolean                                   =  false;
                                delay                                                   (300);
                                justDonePlayingAudioFileBoolean                         =  true;

                                int indexInt                                            =  0;
                                while(repeatInstructionCharPointerQueueList .isEmpty()  == false) {
                                    repeatInstructionCharPointerArray[indexInt]         =  repeatInstructionCharPointerQueueList.pop();
                                    indexInt                                            ++;
                                }
                    }

                    if      (inputString == "I"){ /*CAPTURE_START*/

                                stillCapturingBoolean                                   =  true;
                                delay                                                   (300);
                                justDonePlayingAudioFileBoolean                         =  true;

                                initialBoolean                                          =  false;
                                exhibitionTargetFillInCounterInt                        =  0;
                                while(repeatInstructionCharPointerQueueList .isEmpty()  == false) { repeatInstructionCharPointerQueueList.pop(); }

                    }

                    for     (int i = audioFileIndexMinInt; i <= audioFileIndexMaxInt; i ++){

                                if(
                                    (inputString                                        == String(i)    ) &&
                                    (justDonePlayingAudioFileBoolean                    == false        ) &&
                                    (stillCapturingBoolean                              == true         )
                                ){

                                    vs1053Object.GetAdafruitVS1053FilePlayer()          .playFullFile(AudioIndexCharArray(i));


                                         if     (exhibitionTargetFillInCounterInt       == 0){

                                            exhibitionExplanationIndexInt               = i;
                                            exhibitionExplanationIndexString            = String        (exhibitionExplanationIndexInt) + ".mp3";
                                            exhibitionExplanationIndexString            .toCharArray    (exhibitionExplanationIndexCharArray, 10);
                                            repeatInstructionCharPointerQueueList       .push           (exhibitionExplanationIndexCharArray);

                                    }
                                    else if     (exhibitionTargetFillInCounterInt       == 1){

                                            exhibitionTargetIndex1Int                   = i;
                                            exhibitionTargetIndex1String                = String        (exhibitionTargetIndex1Int) + ".mp3";
                                            exhibitionTargetIndex1String                .toCharArray    (exhibitionTargetIndex1CharArray, 10);
                                            repeatInstructionCharPointerQueueList       .push           (exhibitionTargetIndex1CharArray);

                                    }
                                    else if     (exhibitionTargetFillInCounterInt       == 2){

                                            exhibitionTargetIndex2Int                   = i;
                                            exhibitionTargetIndex2String                = String        (exhibitionTargetIndex2Int) + ".mp3";
                                            exhibitionTargetIndex2String                .toCharArray    (exhibitionTargetIndex2CharArray, 10);
                                            repeatInstructionCharPointerQueueList       .push           (exhibitionTargetIndex2CharArray);

                                    }
                                    else if     (exhibitionTargetFillInCounterInt       == 3){

                                            exhibitionTargetIndex3Int                   = i;
                                            exhibitionTargetIndex3String                = String        (exhibitionTargetIndex3Int) + ".mp3";
                                            exhibitionTargetIndex3String                .toCharArray    (exhibitionTargetIndex3CharArray, 10);
                                            repeatInstructionCharPointerQueueList       .push           (exhibitionTargetIndex3CharArray);

                                    }

                                    justDonePlayingAudioFileBoolean                     = true;
                                    exhibitionTargetFillInCounterInt                    ++;

                                }
                    }

                }

}
void        EstablishProcessingConnection                                       (){ while(Serial.available() <= 0){ Serial.println("HANDSHAKE"); delay(300); } }
/*Create a function that send CharArray to a connection between Arduino and the Processing sketch.*/
void        MoveExhibitionVoid                                                  (){

                if(exhibitionReceiveIndexInt != -1){
                    Serial.println  (rfidReaderObject.GetExhibitionReceivedNameAltString());
                    delay           (300);
                    /*
                    Serial.println  ("SENT_PLAYER_IND_XXX="                     + String(playerIndexInt)                                );
                    delay           (300);
                    Serial.println  ("SENT_PLAYER_EXH_NXT="                     + rfidReaderObject.GetExhibitionReceivedNameAltString() );
                    delay           (300);
                    */
                }

}
void        PlayNextVoid                                                        (bool _playNoInterruptionBool){

                    audioFileIndexInt                                           ++;
                     if( audioFileIndexInt                                      >  audioFileIndexMaxInt)    { audioFileIndexInt = audioFileIndexMinInt; }
                     if(_playNoInterruptionBool                                 == true )                   { vs1053Object.GetAdafruitVS1053FilePlayer().playFullFile       (AudioIndexCharArray(audioFileIndexInt)); }
                else if(_playNoInterruptionBool                                 == false)                   { vs1053Object.GetAdafruitVS1053FilePlayer().startPlayingFile   (AudioIndexCharArray(audioFileIndexInt)); }

}
void        PlayPreviousVoid                                                    (bool _playNoInterruptionBool){

                    audioFileIndexInt                                           --;
                     if( audioFileIndexInt                                      <  audioFileIndexMinInt)    { audioFileIndexInt = audioFileIndexMaxInt; }
                     if(_playNoInterruptionBool                                 == true )                   { vs1053Object.GetAdafruitVS1053FilePlayer().playFullFile       (AudioIndexCharArray(audioFileIndexInt)); }
                else if(_playNoInterruptionBool                                 == false)                   { vs1053Object.GetAdafruitVS1053FilePlayer().startPlayingFile   (AudioIndexCharArray(audioFileIndexInt)); }

}
void        RepeatInstructionVoid                                               (boolean _initialBoolean){

                QueueList<char*>                    tempRepeatInstruction1CharPointerStackList  = repeatInstructionCharPointerQueueList ;
                     if(_initialBoolean             == true ){
                        vs1053Object                .GetAdafruitVS1053FilePlayer().playFullFile (audioFileWelcomeToCharArray);
                        vs1053Object                .GetAdafruitVS1053FilePlayer().playFullFile (audioFileSampleCharArray);
                        vs1053Object                .GetAdafruitVS1053FilePlayer().playFullFile (audioFileMuseumCharArray);
                        vs1053Object                .GetAdafruitVS1053FilePlayer().playFullFile (audioFilePleaseVisitAndTapCharArray);
                        vs1053Object                .GetAdafruitVS1053FilePlayer().playFullFile (audioFileAnyCharArray);
                        vs1053Object                .GetAdafruitVS1053FilePlayer().playFullFile (audioFileExhibitionCharArray);
                }
                else if(_initialBoolean == false){

                        for(int i = 0; i < sizeof(repeatInstructionCharPointerArray); i ++){
                            vs1053Object            .GetAdafruitVS1053FilePlayer().playFullFile (repeatInstructionCharPointerArray[i]);
                        }

                }

}
char*       AudioIndexCharArray                                                 (int  _audioFileIndexInt){

                audioFileIndexInt                                               = _audioFileIndexInt;
                String audioFileIndexString                                     = String(audioFileIndexInt) + ".mp3";
                audioFileIndexString                                            .toCharArray(audioFileIndexCharArray, 10);
                return                                                          audioFileIndexCharArray;

}


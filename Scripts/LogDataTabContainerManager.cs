public class LogDataTabContainerManager : MonoBehaviour {
	public Text logName;
	public Text logSender;
	public Text logSubject;
	public Image logImage;

	void SendLogData(int referenceIndex, bool isRH) {
		//DualLog("SendLogData received referenceIndex of " + referenceIndex.ToString());
		logName.text = Const.a.audiologNames[referenceIndex];
		if (!isRH) {
			logSender.text = Sys_Text.stringTable[893] + Const.a.audiologSenders[referenceIndex];
			logSubject.text = Sys_Text.stringTable[894] + Environment.NewLine + Const.a.audiologSubjects[referenceIndex];
			logImage.overrideSprite = logImages + Const.a.audioLogImagesRefIndicesLH[referenceIndex];
		} else {
			logSender.text = System.String.Empty; // blank on RH side
			logSubject.text = System.String.Empty; // blank on RH side
			logImage.overrideSprite = logImages + Const.a.audioLogImagesRefIndicesRH[referenceIndex];
		}
	}
}

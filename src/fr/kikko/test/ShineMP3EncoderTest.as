package fr.kikko.test {
	import assets.GUI;

	import fr.kikko.lab.ShineMP3Encoder;

	import flash.display.Sprite;
	import flash.events.ErrorEvent;
	import flash.events.Event;
	import flash.events.MouseEvent;
	import flash.events.ProgressEvent;
	import flash.net.FileReference;

	/**
	 * @author kikko.fr
	 */
	public class ShineMP3EncoderTest extends Sprite {
	
		private var mp3Encoder:ShineMP3Encoder;
		private var wavLoader:FileReference;
		private var gui:GUI;
		
		public function ShineMP3EncoderTest() {
						
			wavLoader = new FileReference();
			wavLoader.addEventListener(Event.SELECT, wavSelected);			wavLoader.addEventListener(Event.COMPLETE, wavLoaded);
			
			initGUI();
			
			log("Shine MP3 Encoder on Alchemy Demo");
			log("Please select a 16bit WAV (mono or stereo)");		}

		private function initGUI() : void {
			
			gui = new GUI();
			
			gui.btSave.enabled = false;
			gui.btSave.addEventListener(MouseEvent.MOUSE_DOWN, saveClicked);
			gui.btEncode.enabled = false;
			gui.btEncode.addEventListener(MouseEvent.MOUSE_DOWN, encodeClicked);
			gui.btWav.addEventListener(MouseEvent.MOUSE_DOWN, loadClicked);
			gui.progressBar.mode = "manual";
			
			addChild(gui);
		}

		private function saveClicked(event : MouseEvent) : void {
			
			mp3Encoder.saveAs();
		}

		private function loadClicked(event : MouseEvent) : void {
			
			wavLoader.browse();
		}

		private function wavSelected(event : Event) : void {
			
			wavLoader.load();
			gui.btSave.enabled = false;
			
			log(wavLoader.name, "selected");
		}

		private function wavLoaded(event : Event) : void {
						
			gui.btEncode.enabled = true;
		}

		private function encodeClicked(event:Event):void {
			
			log("encoding...");
			
			mp3Encoder = new ShineMP3Encoder(wavLoader.data);
			mp3Encoder.addEventListener(Event.COMPLETE, mp3EncodeComplete);			mp3Encoder.addEventListener(ProgressEvent.PROGRESS, mp3EncodeProgress);
			mp3Encoder.addEventListener(ErrorEvent.ERROR, mp3EncodeError);
			mp3Encoder.start();
		}

		private function mp3EncodeProgress(event : ProgressEvent) : void {
			
			gui.percent.text = event.bytesLoaded + "%";
			gui.progressBar.setProgress(event.bytesLoaded, event.bytesTotal);
		}

		private function mp3EncodeError(event : ErrorEvent) : void {
			
			log("[ERROR] : ", event.text);
		}
		
		private function log(...args):void {
			
			gui.console.appendText(args.join(" ")+"\n");
			gui.console.textField.scrollV = gui.console.textField.maxScrollV;
		}

		private function mp3EncodeComplete(event : Event) : void {
			
			log("done!");
			gui.btSave.enabled = true;
		}
	}
}

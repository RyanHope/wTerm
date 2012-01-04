enyo.kind({
	name: 'wi.InputKey',
	kind: 'Item',
	
	recording: false,
	lastValue: {},
	currentValue: {},
	
	published: {
		caption: '',
		value: {}
	},
	
	components: [
		
		{name: 'container', className: 'enyo-input', components: [
		
			{className: 'position', kind: 'HFlexBox', components: [
				{name: 'display', allowHtml: true, flex: 1},
				{name: 'input', kind: 'Input', className: 'hidden-input', onblur: 'stopRecording', onfocus: 'startRecording', onkeydown: 'keyDown'},
				{name: 'caption'},
			]},
			
		]},
		
		
	],
	
	create: function () {
	    this.inherited(arguments);
		this.addClass('wi-input-key');
		this.$.caption.setContent(this.caption);
		this.updateDisplay(this.value);
	},
	
	doClick: function(inSender, inEvent) {
		if (!this.recording) {
			this.$.input.forceFocus();
		}
	},
	
	startRecording: function() {
		this.$.container.addClass('enyo-input-focus');
		this.currentValue = this.value;
		this.lastValue = this.value;
		this.updateDisplay(this.currentValue);
		this.recording = true;
	},
	stopRecording: function() {
		this.$.container.removeClass('enyo-input-focus')
		if (this.validValue(this.currentValue)) this.value = this.currentValue;
		else this.value = this.lastValue;
		this.updateDisplay(this.value);
		this.currentValue = {};
		this.lastValue = {};
		this.recording = false;
	},
	
	setValue: function(value) {
		this.value = value;
		this.updateDisplay(this.value);
	},
	getValue: function() {
		return this.value;
	},
	
	keyDown: function(inSender, inEvent) {
		inEvent.preventDefault();
		if (this.recording) {
			this.currentValue = this.getValueFromEvent(inEvent);
			this.updateDisplay(this.currentValue);
			//enyo.application.k.logEvent(inEvent);
		}
		return false;
	},
	
	getValueFromEvent: function(event) {
		var value = {
			keyCode:		event.keyCode,
			keyIdentifier:	event.keyIdentifier,
			ctrlKey:		event.ctrlKey,
			altKey:			event.altKey,
			shiftKey:		event.shiftKey, // if the capslock is on this is reversed!
			metaKey:		event.metaKey
		};
		return value;
	},
	
	updateDisplay: function(value) {
		this.$.display.setContent(this.getStringFromValue(value));
		if (this.validValue(value)) this.$.display.applyStyle('opacity', null);
		else this.$.display.applyStyle('opacity', '0.6');
	},
	
	validValue: function(value) {
		// we will require one modifier and a valid key
		var mods = 0;
		if (value.ctrlKey)	mods++;
		if (value.altKey)	mods++;
		if (value.shiftKey)	mods++;
		if (value.metaKey)	mods++;
		if (mods > 0 &&
			value.keyCode &&		// this list probably needs more
			value.keyCode != 8 &&	// backspace
			value.keyCode != 17 &&	// ctrl
			value.keyCode != 129 &&	// alt
			value.keyCode != 16 &&	// shift
			value.keyCode != 13) {	// enter
			return true;
		}
		else if (value.keyCode == 9) return true // actually, we'll let tab pass without any mods
		else return false;
	},
	
	getStringFromValue: function(value) {
		var used = [];
		if (value.ctrlKey)	used.push('<span class="key">Ctrl</span>');
		if (value.altKey)	used.push('<span class="key">Alt</span>');
		if (value.shiftKey)	used.push('<span class="key">Shft</span>');
		if (value.metaKey)	used.push('<span class="key">Meta</span>');
		if (value.keyCode &&		// this list probably needs more
			value.keyCode != 8 &&	// backspace
			value.keyCode != 17 &&	// ctrl
			value.keyCode != 129 &&	// alt
			value.keyCode != 16 &&	// shift
			value.keyCode != 13) {	// enter
			if (value.keyCode == 9) used.push('<span class="key">Tab</span>');
			else if (value.keyCode == 32) used.push('<span class="key">Space</span>');
			else if (value.keyCode >= 37 && value.keyCode <= 40) used.push('<span class="key">' + value.keyIdentifier + '</span>');
			else used.push('<span class="key">' + String.fromCharCode(value.keyCode) + '</span>');
			//used.push('(' + value.keyCode + ') ' + value.keyIdentifier);
		}
		else
			used.push('<span class="key" style="opacity: 0.3;">&nbsp;</span>');
		
		var pretty = used.join(' + ');
		if (pretty == '') {
			if (this.recording) pretty = 'Press and Hold Keys';
			else pretty = 'None';
		}
		return pretty;
	},
	
});

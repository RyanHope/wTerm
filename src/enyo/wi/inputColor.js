enyo.kind({
	name: 'wi.InputColor',
	kind: 'Item',
	layoutKind: 'HFlexLayout',
	align: 'center',
	
	events: {
		onChanged: ''
	},
	
	published: {
		// caption of the row
		caption: '',

		// initial value
		value: 'rgba(0, 0, 0, 0)',

		// allow user to manually enter a value?
		manualInput: true
	},

	// format is automatically set when you initially set the value at creation or through setValue.
	// valid formats: any valid css color string, an array [r, g, b], or an object {r: 0, g: 0, b: 0}
	format: 'string',
	
	components: [
		
		{name: 'popup', kind: 'wi.InputColor.Popup', onColorSelect: 'colorSelected'},
		
		{flex: 1, components: [
			{name: 'display', className: 'value-display', onclick: 'openPopup', components: [
				{name: 'displayText1', className: 'value-display-text-1'},
				{name: 'displayText2', className: 'value-display-text-2'}
			]},
		]},
		{name: 'caption'},
	],
	
	create: function () {
	    this.inherited(arguments);
		this.addClass('wi-input-color-item');
		this.$.caption.setContent(this.caption);
		this.updateDisplay();
	},
	
	setValue: function(value) {
		this.value = value;
		this.updateDisplay();
	},
	getValue: function() {
		return this.value;
	},
	
	updateDisplay: function() {
		if (enyo.isString(this.value)) {
			this.format = 'string';
			this.$.display.applyStyle('background-color', this.value);
			this.$.displayText1.setContent(this.value);
			this.$.displayText2.setContent(this.value);
		}
		else if (enyo.isArray(this.value)) {
			if (this.value.length == 3 && this.value[0] >= 0 && this.value[1] >= 0 && this.value[2] >= 0) {
				this.format = 'array';
				this.$.display.applyStyle('background-color', 'rgb(' + this.value[0] + ', ' + this.value[1] + ', ' + this.value[2] + ')');
				this.$.displayText1.setContent(this.value[0] + ' / ' + this.value[1] + ' / ' + this.value[2]);
				this.$.displayText2.setContent(this.value[0] + ' / ' + this.value[1] + ' / ' + this.value[2]);
			}
		}
		else if (typeof(this.value) == 'object') {
			if (this.value.r >= 0 && this.value.g >= 0 && this.value.b >= 0) {
				this.format = 'object';
				this.$.display.applyStyle('background-color', 'rgb(' + this.value.r + ', ' + this.value.g + ', ' + this.value.b + ')');
				this.$.displayText1.setContent(this.value.r + ' / ' + this.value.g + ' / ' + this.value.b);
				this.$.displayText2.setContent(this.value.r + ' / ' + this.value.g + ' / ' + this.value.b);
			}
		}
		else {
			this.log('unknown', typeof(this.value), this.value);
			this.format = 'unknown';
			this.$.display.applyStyle('background-color', '#FF0000');
			this.$.displayText1.setContent('[ERR]');
			this.$.displayText2.setContent('[ERR]');
		}
	},
	
	openPopup: function() {
		this.$.popup.openAtCenter();
	},
	
	colorSelected: function(inSender, inColor) {
		this.log(inColor);
		this.value = inColor;
		this.updateDisplay();
		this.doChanged();
	},
	
});

enyo.kind({
	name: 'wi.InputColor.Popup',
	kind: 'Popup',
	scrim: true,
	
	popupBorderWidth:	24,	// so says the css for the popup
	previewWidth:		55,	// the width+marginright of the preview element
	
	events: {
		onColorSelect: ''
	},
	
	components: [
		{kind: 'VFlexBox', className: 'left-container', components: [
			{name: 'original', className: 'original', flex: 1},
			{name: 'preview', className: 'preview', flex: 1},
			{name: 'manualButton', kind: 'Button', className: 'manual-toggle', toggling: true, onclick: 'toggleManual', components: [
				{className: 'icon'}
			]}
		]},
		{
			name: 'canvas',
			kind: enyo.Control,
			className: 'canvas',
			nodeTag: 'canvas',
			domAttributes: {
				width: '340px',
				height: '200px',
			},
			onclick: 'canvasClick',
		},
		{name: 'manual', className: 'manual-container'/*, showing: false*/, components: [
			{name: 'stringFormat', showing: false, kind: 'RowGroup', className: 'manual-group', caption: 'Manual Color Entry', components: [
				{name: 'manualInput', kind: 'Input', hint: 'Any Valid CSS3 Color Unit...',
					autocorrect: false, autoCapitalize: 'lowercase', autoWordComplete: false, selectAllOnFocus: true,
					changeOnInput: true, onkeydown: 'keyDown', onkeyup: 'keyUp', components: [
						{name: 'manualSave', kind: 'CustomButton', className: 'manual-save', onclick: 'manualSave', content: ' '},
				]}
			]},
			{name: 'tripFormat', showing: false, kind: 'RowGroup', className: 'manual-group', caption: 'Manual Color Entry', components: [
				{content: '...some sort of rgb 0-255 selector...'}
				//{kind: "IntegerPicker", label: "rating", min: 0, max: 255, onChange: "pickerChange"}
			]},
		]},
	],
	
	componentsReady: function() {
		this.inherited(arguments);
		if (this.owner.manualInput) {
			switch (this.owner.format) {
				case 'string':
					this.$.stringFormat.setShowing(true);
					break;
				case 'array':
				case 'object':
					this.$.tripFormat.setShowing(true);
					break;
			}
		}
		else {
			this.$.manualButton.setShowing(false);
		}
		this.addClass('wi-input-color-popup');
		//this.parent.applyStyle('-webkit-perspective', '768px'); // Y U NO WORK?
		//this.applyStyle('-webkit-transform-style', 'preserve-3d');
		//this.applyStyle('-webkit-transition', '-webkit-transform 0.3s ease');
		this.$.canvas.applyStyle('-webkit-transition', 'opacity 0.6s linear');
	},
	
	doOpen: function() {
		this.$.canvas.hasNode();
		this.ctx = this.$.canvas.node.getContext('2d');
		
		this.img = new Image();
		this.img.src = 'src/enyo/wi/images/colors.png';
		this.img.onload = enyo.bind(this, 'drawImage');
		
		switch (this.owner.format) {
			case 'string':
				this.$.original.applyStyle('background-color', this.owner.value);
				this.$.preview.applyStyle('background-color',  this.owner.value);
				this.$.manualInput.setValue(this.owner.value);
				break;
			case 'array':
				this.$.original.applyStyle('background-color', 'rgb(' + this.owner.value[0] + ', ' + this.owner.value[1] + ', ' + this.owner.value[2] + ')');
				this.$.preview.applyStyle('background-color',  'rgb(' + this.owner.value[0] + ', ' + this.owner.value[1] + ', ' + this.owner.value[2] + ')');
				break;
			case 'object':
				this.$.original.applyStyle('background-color', 'rgb(' + this.owner.value.r + ', ' + this.owner.value.g + ', ' + this.owner.value.b + ')');
				this.$.preview.applyStyle('background-color',  'rgb(' + this.owner.value.r + ', ' + this.owner.value.g + ', ' + this.owner.value.b + ')');
				break;
		}
		this.$.manualButton.setDepressed(false);
		this.hideManual();
	},
	doClose: function() {
	},
	
	drawImage: function() {
		this.ctx.drawImage(this.img, 0, 0);
	},
	
	canvasClick: function(inSender, inEvent) {
		if (!this.manualShowing) {
			var d = this.canvasGetColorFromPosition(this.canvasCursorClickPosition(inEvent));
			switch (this.owner.format) {
				case 'string':
					var c = this.rgbToHex(d);
					break;
				case 'array':
					var c = [d.r, d.g, d.b];
					break;
				case 'object':
					var c = {r: d.r, g: d.g, b: d.b};
					break;
			}
			this.doColorSelect(c);
			this.close();
		}
		else {
			this.hideManual();
		}
	},
	canvasCursorClickPosition: function(inEvent) {
		var x, y;
		x = inEvent.offsetX;
		y = inEvent.offsetY;
		return {x: x, y: y};
	},
	canvasGetColorFromPosition: function(pos) {
		var data = this.ctx.getImageData(pos.x, pos.y, 1, 1).data;
		return {r: data[0], g: data[1], b: data[2], a: data[3]};
	},
	
	dragGetCanvasPosition: function(inSender, inEvent) {
		var x, y;
		x = inEvent.pageX - (parseInt(this.node.style.left, 10) + this.popupBorderWidth + this.previewWidth);
		y = inEvent.pageY - (parseInt(this.node.style.top, 10) + this.popupBorderWidth);
		return {x: x, y: y};
	},
	dragstartHandler: function(inSender, inEvent) {
	},
	dragHandler: function(inSender, inEvent) {
		if (!this.manualShowing) {
			var c = this.canvasGetColorFromPosition(this.dragGetCanvasPosition(inSender, inEvent));
			this.$.preview.applyStyle('background-color', 'rgb(' + c.r + ', ' + c.g + ', ' + c.b + ')');
		}
	},
	dragfinishHandler: function(inSender, inEvent) {
		if (!this.manualShowing) {
			var d = this.canvasGetColorFromPosition(this.canvasCursorClickPosition(inEvent));
			switch (this.owner.format) {
				case 'string':
					var c = this.rgbToHex(d);
					break;
				case 'array':
					var c = [d.r, d.g, d.b];
					break;
				case 'object':
					var c = {r: d.r, g: d.g, b: d.b};
					break;
			}
			var c = this.rgbToHex(this.canvasGetColorFromPosition(this.dragGetCanvasPosition(inSender, inEvent)));
			this.doColorSelect(c);
		}
	},
	
	toggleManual: function(inSender, inEvent) {
		if (this.manualShowing) {
			this.hideManual();
			//this.applyStyle('-webkit-transform', 'rotateY(0deg)');
		}
		else {
			this.showManual();
			//this.applyStyle('-webkit-transform', 'rotateY(180deg)');
		}
	},
	hideManual: function() {
		this.manualShowing = false;
		this.$.canvas.applyStyle('opacity', '1');
		this.$.manual.hide();
		this.$.manualButton.setDepressed(false);
	},
	showManual: function() {
		this.manualShowing = true;
		this.keyUp();
		this.$.canvas.applyStyle('opacity', '0.3');
		this.$.manual.show();
		this.$.manualInput.forceFocus();
	},
	
	manualSave: function(inSender, inEvent) {
		var text = this.$.manualInput.getValue();
		this.doColorSelect(text);
		this.close();
	},
	
	keyDown: function(inSender, inEvent) {
		var text = this.$.manualInput.getValue();
		if (inEvent.keyCode === 13) {
			inEvent.preventDefault();
			if (text && this.isValidColorString(text)) {
				this.doColorSelect(text);
				this.close();
			}
		}
	},
	keyUp: function(inSender, inEvent) {
		var text = this.$.manualInput.getValue();
		if (this.isValidColorString(text)) {
			this.$.preview.applyStyle('background-color', text);
			this.$.manualInput.$.input.applyStyle('color', null);
			this.$.manualSave.setDisabled(false);
		}
		else {
			this.$.manualInput.$.input.applyStyle('color', 'rgba(0, 0, 0, 0.6)');
			this.$.manualSave.setDisabled(true);
		}
	},
	
	isValidColorString: function(string) {
		if (colorWords.indexOf(string) > -1) return true;
		if (string.match(/#([a-fA-F0-9]{3}|[a-fA-F0-9]{6})$/)) return true;
		if (string.match(/rgb\(\s*\b([0-9]|[1-9][0-9]|1[0-9][0-9]|2[0-4][0-9]|25[0-5])\b\s*,\s*\b([0-9]|[1-9][0-9]|1[0-9][0-9]|2[0-4][0-9]|25[0-5])\b\s*,\s*\b([0-9]|[1-9][0-9]|1[0-9][0-9]|2[0-4][0-9]|25[0-5])\b\s*\)/)) return true;
		if (string.match(/rgba\(\s*\b([0-9]|[1-9][0-9]|1[0-9][0-9]|2[0-4][0-9]|25[0-5])\b\s*,\s*\b([0-9]|[1-9][0-9]|1[0-9][0-9]|2[0-4][0-9]|25[0-5])\b\s*,\s*\b([0-9]|[1-9][0-9]|1[0-9][0-9]|2[0-4][0-9]|25[0-5])\b\s*,\s*([0-9]+(?:\.[0-9]+)?|\.[0-9]+)\s*\)/)) return true;
		if (string.match(/rgb\(\s*(\d?\d%|100%)+\s*,\s*(\d?\d%|100%)+\s*,\s*(\d?\d%|100%)+\s*\)/)) return true;
		if (string.match(/rgba\(\s*(\d?\d%|100%)+\s*,\s*(\d?\d%|100%)+\s*,\s*(\d?\d%|100%)+\s*,\s*([0-9]+(?:\.[0-9]+)?|\.[0-9]+)\s*\)/)) return true;
		if (string.match(/hsl\(\s*\b([0-9]|[1-9][0-9]|1[0-9][0-9]|2[0-9][0-9]|3[0-5][0-9]|360)\b\s*,\s*(\d?\d%|100%)+\s*,\s*(\d?\d%|100%)+\s*\)/)) return true;
		if (string.match(/hsla\(\s*\b([0-9]|[1-9][0-9]|1[0-9][0-9]|2[0-9][0-9]|3[0-5][0-9]|360)\b\s*,\s*(\d?\d%|100%)+\s*,\s*(\d?\d%|100%)+\s*,\s*([0-9]+(?:\.[0-9]+)?|\.[0-9]+)\s*\)/)) return true;
		return false;
	},
	
	rgbToHex: function(t) {
		return '#' + this.toHex(t.r) + this.toHex(t.g) + this.toHex(t.b);
	},
	toHex: function(n) {
		 n = parseInt(n,10);
		 if (isNaN(n)) return "00";
		 n = Math.max(0, Math.min(n,255));
		 return "0123456789ABCDEF".charAt((n-n%16)/16)
		      + "0123456789ABCDEF".charAt(n%16);
	},
	
});

var colorWords = [
	'aliceblue',
	'antiquewhite',
	'aqua',
	'aquamarine',
	'azure',
	'beige',
	'bisque',
	'black',
	'blanchedalmond',
	'blue',
	'blueviolet',
	'brown',
	'burlywood',
	'cadetblue',
	'chartreuse',
	'chocolate',
	'coral',
	'cornflowerblue',
	'cornsilk',
	'crimson',
	'cyan',
	'darkblue',
	'darkcyan',
	'darkgoldenrod',
	'darkgray',
	'darkgreen',
	'darkkhaki',
	'darkmagenta',
	'darkolivegreen',
	'darkorange',
	'darkorchid',
	'darkred',
	'darksalmon',
	'darkseagreen',
	'darkslateblue',
	'darkslategray',
	'darkturquoise',
	'darkviolet',
	'deeppink',
	'deepskyblue',
	'dimgray',
	'dodgerblue',
	'firebrick',
	'floralwhite',
	'forestgreen',
	'fuchsia',
	'gainsboro',
	'ghostwhite',
	'gold',
	'goldenrod',
	'gray',
	'green',
	'greenyellow',
	'honeydew',
	'hotpink',
	'indianred',
	'indigo',
	'ivory',
	'khaki',
	'lavender',
	'lavenderblush',
	'lawngreen',
	'lemonchiffon',
	'lightblue',
	'lightcoral',
	'lightcyan',
	'lightgoldenrodyellow',
	'lightgreen',
	'lightpink',
	'lightsalmon',
	'lightseagreen',
	'lightskyblue',
	'lightslategray',
	'lightsteelblue',
	'lightyellow',
	'lime',
	'limegreen',
	'linen',
	'magenta',
	'maroon',
	'mediumaquamarine',
	'mediumblue',
	'mediumorchid',
	'mediumpurple',
	'mediumseagreen',
	'mediumslateblue',
	'mediumspringgreen',
	'mediumturquoise',
	'mediumvioletred',
	'midnightblue',
	'mintcream',
	'mistyrose',
	'moccasin',
	'navajowhite',
	'navy',
	'oldlace',
	'olive',
	'olivedrab',
	'orange',
	'orangered',
	'orchid',
	'palegoldenrod',
	'palegreen',
	'paleturquoise',
	'palevioletred',
	'papayawhip',
	'peachpuff',
	'peru',
	'pink',
	'plum',
	'powderblue',
	'purple',
	'red',
	'rosybrown',
	'royalblue',
	'saddlebrown',
	'salmon',
	'sandybrown',
	'seagreen',
	'seashell',
	'sienna',
	'silver',
	'skyblue',
	'slateblue',
	'slategray',
	'snow',
	'springgreen',
	'steelblue',
	'tan',
	'teal',
	'thistle',
	'tomato',
	'turquoise',
	'violet',
	'wheat',
	'white',
	'whitesmoke',
	'yellow',
	'yellowgreen'
];

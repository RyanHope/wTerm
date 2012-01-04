enyo.kind({
	name: "Preferences",
	kind: enyo.Toaster,
	events: {
		onBypassClose: ""
	},
	published : {
		terminal: null
	},
	lazy: false,
	components: [
		{name: "shadow", className: "enyo-sliding-view-shadow"},
		{kind: "VFlexBox", height: "100%", components: [
			{kind: "Header", pack: 'center', content: "Preferences"},
	  		{kind: "Scroller", flex: 1, components: [
	  			{kind: "RowGroup", caption: 'Text', flex: 1, components: [
					{kind: 'Item', layoutKind: 'HFlexLayout', align: "center", style: 'padding: 0; margin: 0', components: [
						{kind: "IntegerPicker", name: 'fontSize', label: '', min: 8, max: 22, flex: 1, onChange: 'fontSizeChanged'},
						{content: "Font Size", style: 'padding-right: 10px'}
					]}
				]},
				{kind: 'RowGroup', flex :1, caption: 'Color Scheme', components: [
					{name: 'foreground', kind: 'wi.InputColor', caption: 'Foreground', onChanged: 'updateColors'},
					{name: 'background', kind: 'wi.InputColor', caption: 'Background', onChanged: 'updateColors'},					
					{name: 'color1', kind: 'wi.InputColor', caption: 'Color1', onChanged: 'updateColors'},
					{name: 'color2', kind: 'wi.InputColor', caption: 'Color2', onChanged: 'updateColors'},
					{name: 'color3', kind: 'wi.InputColor', caption: 'Color3', onChanged: 'updateColors'},
					{name: 'color4', kind: 'wi.InputColor', caption: 'Color4', onChanged: 'updateColors'},
					{name: 'color5', kind: 'wi.InputColor', caption: 'Color5', onChanged: 'updateColors'},
					{name: 'color6', kind: 'wi.InputColor', caption: 'Color6', onChanged: 'updateColors'},
					{name: 'color7', kind: 'wi.InputColor', caption: 'Color7', onChanged: 'updateColors'},
					{name: 'color8', kind: 'wi.InputColor', caption: 'Color8', onChanged: 'updateColors'},
					{name: 'foregroundBright', kind: 'wi.InputColor', caption: 'Foreground Bright', onChanged: 'updateColors'},
					{name: 'backgroundBright', kind: 'wi.InputColor', caption: 'Background Bright', onChanged: 'updateColors'},
					{name: 'color1Bright', kind: 'wi.InputColor', caption: 'Color1 Bright', onChanged: 'updateColors'},
					{name: 'color2Bright', kind: 'wi.InputColor', caption: 'Color2 Bright', onChanged: 'updateColors'},
					{name: 'color3Bright', kind: 'wi.InputColor', caption: 'Color3 Bright', onChanged: 'updateColors'},
					{name: 'color4Bright', kind: 'wi.InputColor', caption: 'Color4 Bright', onChanged: 'updateColors'},
					{name: 'color5Bright', kind: 'wi.InputColor', caption: 'Color5 Bright', onChanged: 'updateColors'},
					{name: 'color6Bright', kind: 'wi.InputColor', caption: 'Color6 Bright', onChanged: 'updateColors'},
					{name: 'color7Bright', kind: 'wi.InputColor', caption: 'Color7 Bright', onChanged: 'updateColors'},
					{name: 'color8Bright', kind: 'wi.InputColor', caption: 'Color8 Bright', onChanged: 'updateColors'},
				]},
	  		]},
			{kind: "Toolbar", className: 'enyo-toolbar-light', align: "center", showing: true, components: [
				{name: "dragHandle", kind: "GrabButton", onclick: "close"},
				{kind: 'Button', caption: 'Restore Defaults', disabled: true}
			]}
		]}
	],
	flyInFromChanged: function() {
		this.inherited(arguments);
		this.$.shadow.addRemoveClass("flyInFromLeft", this.flyInFrom == "left");
		this.$.dragHandle.addRemoveClass("flyInFromLeft", this.flyInFrom == "left");
	},
	findZIndex: function() {
		// we want to make sure this is on top of any other popups, even popups that open after this is opened.
		return 400;
	},
	close: function(e, reason) {
		if (!this.doBypassClose(e)) {
			this.inherited(arguments);
		}
	},
	updateColors: function() {
		var colors = [
			this.$.color1.getValue(),
			this.$.color2.getValue(),
			this.$.color3.getValue(),
			this.$.color4.getValue(),
			this.$.color5.getValue(),
			this.$.color6.getValue(),
			this.$.color7.getValue(),
			this.$.color8.getValue(),
			this.$.color1Bright.getValue(),
			this.$.color2Bright.getValue(),
			this.$.color3Bright.getValue(),
			this.$.color4Bright.getValue(),
			this.$.color5Bright.getValue(),
			this.$.color6Bright.getValue(),
			this.$.color7Bright.getValue(),
			this.$.color8Bright.getValue(),
			this.$.foreground.getValue(),
			this.$.background.getValue(),
			this.$.foregroundBright.getValue(),
			this.$.backgroundBright.getValue(),
		]
		this.prefs.set('colors', colors)
		this.terminal.setColors()
	},
	getColors: function() {
		var colors = this.prefs.get('colors')
		this.$.color1.setValue(colors[0])
		this.$.color2.setValue(colors[1])
		this.$.color3.setValue(colors[2])
		this.$.color4.setValue(colors[3])
		this.$.color5.setValue(colors[4])
		this.$.color6.setValue(colors[5])
		this.$.color7.setValue(colors[6])
		this.$.color8.setValue(colors[7])
		this.$.color1Bright.setValue(colors[8])
		this.$.color2Bright.setValue(colors[9])
		this.$.color3Bright.setValue(colors[10])
		this.$.color4Bright.setValue(colors[11])
		this.$.color5Bright.setValue(colors[12])
		this.$.color6Bright.setValue(colors[13])
		this.$.color7Bright.setValue(colors[14])
		this.$.color8Bright.setValue(colors[15])
		this.$.foreground.setValue(colors[16])
		this.$.background.setValue(colors[17])
		this.$.foregroundBright.setValue(colors[18])
		this.$.backgroundBright.setValue(colors[19])
	},
	fontSizeChanged: function() {
		this.prefs.set('fontSize',this.$.fontSize.value)
	},
	rendered: function() {
		this.getColors()
		this.$.fontSize.setValue(this.prefs.get('fontSize'))
	}
})
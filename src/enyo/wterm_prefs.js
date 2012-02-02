enyo.kind({
	name: "PrefsPullout",
	kind: enyo.Toaster,
	events: {
		onBypassClose: "",
		onVKBLayoutChange: "",
	},
	published : {
		terminal: null
	},
	lazy: false,
	components: [
		{
			kind: 'Popup2',
			name: 'warningwarning',
			modal: true,
			scrim: true,
			dismissWithClick: true,
			components: [
				{style: 'text-align: center; padding-bottom: 12px; font-size: 120%;', allowHtml: true, content: '<b><u>Warning!</u></b>'},
				{allowHtml: true, style: 'text-align: center', content: 'Enabling this option will allow any trojan or virus to execute<br>destructive commands on your device without your knowledge!'},
			]
		},
		{name: "shadow", className: "enyo-sliding-view-shadow"},
		{kind: "VFlexBox", height: "100%", components: [
			{kind: "Header", pack: 'center', components: [
				{kind: "RadioToolButtonGroup", name: 'grp', value: 'appearance', flex: 1, onChange: 'grpChanged', components: [
					{caption: "Appearance", value: 'appearance', flex: 2},
					{caption: "Input", value: 'input'},
					{caption: "Misc", value: 'misc'},
				]}
			]},
			{kind: "Scroller", flex: 1, components: [
				{name: 'grp1', components: [
					{kind: "RowGroup", caption: 'Text', flex: 1, components: [
						{kind: 'Item', layoutKind: 'HFlexLayout', align: "center", style: 'padding: 0; margin: 0', components: [
							{kind: "IntegerPicker", name: 'fontSize', label: '', min: 8, max: 22, flex: 1, onChange: 'fontSizeChanged'},
							{content: "Font Size", style: 'padding-right: 10px', className: 'enyo-label'}
						]}
					]},
					{kind: 'RowGroup', flex :1, caption: 'Color Scheme', components: [
						{kind: "ListSelector", name: 'colorSchemes', onChange: "colorSchemeChanged"},
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
				{name: 'grp2', showing: false, components: [
					{kind: "RowGroup", caption: 'Keyboard Layout', flex: 1, components: [
						{kind: "ListSelector", name: 'kbdLayouts', onChange: "kbdLayoutChanged" },
					]},
					{kind: 'RowGroup', flex :1, caption: 'Key Bindings', components: [
						{kind: "ListSelector", name: 'inputSchemes', onChange: "inputSchemeChanged"},
						{kind: "Input", name: 'inputF1', onChange: 'inputSchemeChanged', components: [{className: 'enyo-label', content: 'F1'}]},
						{kind: "Input", name: 'inputF2', onChange: 'inputSchemeChanged', components: [{className: 'enyo-label', content: 'F2'}]},
						{kind: "Input", name: 'inputF3', onChange: 'inputSchemeChanged', components: [{className: 'enyo-label', content: 'F3'}]},
						{kind: "Input", name: 'inputF4', onChange: 'inputSchemeChanged', components: [{className: 'enyo-label', content: 'F4'}]},
						{kind: "Input", name: 'inputF5', onChange: 'inputSchemeChanged', components: [{className: 'enyo-label', content: 'F5'}]},
						{kind: "Input", name: 'inputF6', onChange: 'inputSchemeChanged', components: [{className: 'enyo-label', content: 'F6'}]},
						{kind: "Input", name: 'inputF7', onChange: 'inputSchemeChanged', components: [{className: 'enyo-label', content: 'F7'}]},
						{kind: "Input", name: 'inputF8', onChange: 'inputSchemeChanged', components: [{className: 'enyo-label', content: 'F8'}]},
						{kind: "Input", name: 'inputF9', onChange: 'inputSchemeChanged', components: [{className: 'enyo-label', content: 'F9'}]},
						{kind: "Input", name: 'inputF10', onChange: 'inputSchemeChanged', components: [{className: 'enyo-label', content: 'F10'}]},
						{kind: "Input", name: 'inputF11', onChange: 'inputSchemeChanged', components: [{className: 'enyo-label', content: 'F11'}]},
						{kind: "Input", name: 'inputF12', onChange: 'inputSchemeChanged', components: [{className: 'enyo-label', content: 'F12'}]},
					]}
				]},
				{name: 'grp3', showing: false, components: [
					{kind: "RowGroup", flex :1, caption: 'Default Exec Command', components: [
						{kind: "Input", name: 'exec', onchange: 'execChanged'},
					]},
					{kind: "RowGroup", flex :1, caption: 'Exhibition Mode Command', components: [
						{kind: "Input", name: 'exhibition', onchange: 'exhibitionChanged'},
					]},
					{kind: "RowGroup", flex :1, caption: 'Just Type Actions', components: [
						{kind: "HFlexBox", components: [
					        {content: "Execute Without Warning", flex: 1},
					        {kind: "ToggleButton", name: 'justType', onChange: 'launchParamWarn', onLabel: 'True', offLabel: 'False'}
					    ]}
					]},
					{kind: "RowGroup", flex :1, caption: 'Scroll Buffer Lines', components: [
						{kind: "Input", name: 'bufferlines', onchange: 'bufferlinesChanged'},
					]},
			]},
			]},
			{kind: "Toolbar", className: 'enyo-toolbar-light', align: "center", showing: true, components: [
				{name: "dragHandle", kind: "GrabButton", onclick: "close"},
				{kind: 'Button', caption: 'Restore Defaults', onclick: "restore"}
			]}
		]}
	],

	restore: function() {
		enyo.application.p.reset()
		this.rendered()
		this.terminal.setFontSize(enyo.application.p.get('fontSize'))
		this.terminal.setColors()
		this.terminal.setKeys()
		this.vkb.loadLayout(this.$.kbdLayouts.getValue());
	},

	flyInFromChanged: function() {
		this.inherited(arguments);
		this.$.shadow.addRemoveClass("flyInFromLeft", this.flyInFrom == "left");
		this.$.dragHandle.addRemoveClass("flyInFromLeft", this.flyInFrom == "left");
	},
	close: function(e, reason) {
		if (!this.doBypassClose(e)) {
			this.inherited(arguments);
		}
	},
	updateColors: function() {
		var colorSchemes = enyo.application.p.get('colorSchemes')
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
		colorSchemes['Custom'] = colors
		enyo.application.p.set('colorSchemes', colorSchemes)
		enyo.application.p.set('colorScheme', 'Custom')
		this.terminal.setColors()
		this.getColorSchemes()
	},
	getKeys: function() {
		var keys = this.terminal.currentKeys
		this.$.inputF1.setValue(keys[0])
		this.$.inputF2.setValue(keys[1])
		this.$.inputF3.setValue(keys[2])
		this.$.inputF4.setValue(keys[3])
		this.$.inputF5.setValue(keys[4])
		this.$.inputF6.setValue(keys[5])
		this.$.inputF7.setValue(keys[6])
		this.$.inputF8.setValue(keys[7])
		this.$.inputF9.setValue(keys[8])
		this.$.inputF10.setValue(keys[9])
		this.$.inputF11.setValue(keys[10])
		this.$.inputF12.setValue(keys[11])
	},
	getColors: function() {
		var colors = this.terminal.currentColors
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
		enyo.application.p.set('fontSize',this.$.fontSize.value)
		this.terminal.setFontSize(enyo.application.p.get('fontSize'))
	},
	getColorSchemes: function() {
		var colorSchemes = enyo.application.p.get('colorSchemes')
		var items = []
		for (var colorScheme in colorSchemes)
			items.push(colorScheme)
		this.$.colorSchemes.setItems(items)
		this.$.colorSchemes.setValue(enyo.application.p.get('colorScheme'))
	},
	getInputSchemes: function() {
		var inputSchemes = enyo.application.p.get('inputSchemes')
		var items = []
		for (var inputScheme in inputSchemes)
			items.push(inputScheme)
		this.$.inputSchemes.setItems(items)
		this.$.inputSchemes.setValue(enyo.application.p.get('inputScheme'))
	},
	exhibitionChanged: function() {
		enyo.application.p.set('exhibition', this.$.exhibition.getValue())
		var windows = enyo.windows.getWindows()
		if (windows['dock']) windows['dock'].close()
	},
	execChanged: function() {
		enyo.application.p.set('exec', this.$.exec.getValue())
	},
	bufferlinesChanged: function() {
		var lines = parseInt(this.$.bufferlines.getValue())
		if (lines>=0) {
			enyo.application.p.set('bufferlines', lines)
			this.terminal.setScrollBufferLines(lines)
		} else {
			this.$.bufferlines.setValue(enyo.application.p.get('bufferlines'))
		}
	},
	launchParamWarn: function() {
		enyo.application.p.set('launchParamsOK', this.$.justType.state)
		if (this.$.justType.state)
			this.$.warningwarning.openAtTopCenter()
	},
	rendered: function() {
		this.getColors()
		this.$.exhibition.setValue(enyo.application.p.get('exhibition'))
		this.$.exec.setValue(enyo.application.p.get('exec'))
		this.$.justType.setState(enyo.application.p.get('launchParamsOK'))
		this.$.bufferlines.setValue(enyo.application.p.get('bufferlines'))
		this.$.fontSize.setValue(enyo.application.p.get('fontSize'))
		this.$.kbdLayouts.setItems(kbdLayoutList());
		this.$.kbdLayouts.setValue(enyo.application.p.get('kbdLayout'))
		this.getColorSchemes()
		this.getKeys()
		this.getInputSchemes()
	},
	colorSchemeChanged: function() {
		enyo.application.p.set('colorScheme', this.$.colorSchemes.getValue())
		this.terminal.setColors()
		this.getColors()
	},
	kbdLayoutChanged: function() {
		enyo.application.p.set('kbdLayout', this.$.kbdLayouts.getValue());
		this.vkb.loadLayout(this.$.kbdLayouts.getValue());
		this.doVKBLayoutChange()
	},
	inputSchemeChanged: function() {
		enyo.application.p.set('inputScheme', this.$.inputSchemes.getValue())
		this.terminal.setKeys()
		this.getKeys()
	},
	grpChanged: function() {
		if (this.$.grp.getValue() == 'appearance') {
			this.$.grp1.setShowing(true)
			this.$.grp2.setShowing(false)
			this.$.grp3.setShowing(false)
		} else if (this.$.grp.getValue() == 'input') {
			this.$.grp2.setShowing(true)
			this.$.grp1.setShowing(false)
			this.$.grp3.setShowing(false)
		} else {
			this.$.grp3.setShowing(true)
			this.$.grp1.setShowing(false)
			this.$.grp2.setShowing(false)
		}
	}
})

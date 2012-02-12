enyo.kind({
	
	name: 'charKey',
	kind: "CustomButton",
	layoutKind: 'HFlexLayout',

	className: 'charKey',

	pack: 'center',
	align: 'center',
	
	published: {
		sym: null,
		unicode: '',
	},

	downChanged: function() {
		this.inherited(arguments);
		if (this.down)
			this.applyStyle('background-color', 'darkgray')
		else
			this.applyStyle('background-color', 'none')
	},
	
	hotChanged: function() {
		this.inherited(arguments);
		if (!this.hot)
			this.applyStyle('background-color', 'none')
	},
	
	setBorder: function(top, bottom, left, right) {
		this.domStyles["border-top-style"] = top
		this.domStyles["border-bottom-style"] = bottom
		this.domStyles["border-left-style"] = left
		this.domStyles["border-right-style"] = right
	},

	setData: function(data) {
		this.setUnicode(data[0])
		this.setCaption(this.getUnicode())
		if (data.length > 1) {
			this.setSym(data[1])
			this.applyStyle('color','#0000FF')
		}
	}

})

enyo.kind({
	
	name: 'CharSelector',
	kind: "Popup",

	className: 'enyo-popup charSelect',

	chars: [
		[ ["F1", SDLK._F1], ["F2", SDLK._F2], ["F3", SDLK._F3], ["F4", SDLK._F4], ["F5", SDLK._F5], ["F6", SDLK._F6], ["F7", SDLK._F7], ["F8", SDLK._F8], ["F9", SDLK._F9], ["F10", SDLK._F10], ["F11", SDLK._F11], ["F12", SDLK._F12]],
		[ ["$"], ["€"], [";"], ["<"], [">"], ["["], ["\\"], ["]"], ["^"], ["`"], ["{"], ["|"], ["}"], ["~"], ["¡"], ["¢"], ["£"], ["¤"], ["¥"], ["§"], ["©"], ["ª"], ["«"], ["¬"], ["®"], ["°"], ["±"], ["º"], ["¹"], ["²"], ["³"], ["µ"], ["¶"], ["»"], ["¼"], ["½"], ["¾"], ["¿"], ["ƒ"], ["‘"], ["’"], ["‚"], ["‛"], ["“"], ["”"], ["„"], ["†"], ["‡"], ["•"], ["…"], ["‰"], ["‹"], ["›"], ["™"], ["Ω"], ["×"], ["÷"], ["Þ"], ["ß"], ["à"], ["á"], ["â"], ["ã"], ["ä"], ["å"], ["æ"], ["ç"], ["ć"], ["è"], ["é"], ["ê"], ["ë"], ["ę"], ["ē"], ["ì"], ["í"], ["î"], ["ï"], ["ł"], ["ð"], ["ñ"], ["ń"], ["ò"], ["ó"], ["ô"], ["õ"], ["ö"], ["ø"], ["ő"], ["œ"], ["š"], ["ù"], ["ú"], ["û"], ["ü"], ["ű"], ["ý"], ["ÿ"], ["ž"], ["ź"], ["ż"], ["þ"], ["À"], ["Á"], ["Â"], ["Ã"], ["Ä"], ["Å"], ["Æ"], ["Ç"], ["Ć"], ["È"], ["É"], ["Ê"], ["Ë"], ["Ę"], ["Ì"], ["Í"], ["Î"], ["Ï"], ["Ł"], ["Ð"], ["Ñ"], ["Ń"], ["Ò"], ["Ó"], ["Ô"], ["Õ"], ["Ö"], ["Ø"], ["Ő"], ["Œ"], ["Š"], ["Ù"], ["Ú"], ["Û"], ["Ü"], ["Ű"], ["Ý"], ["Ÿ"], ["Ž"], ["Ź"], ["Ż"] ],
		[ ["="], ["\""], ["%"], ["_"], ["$"], ["€"], [";"], ["<"], [">"], ["["], ["\\"], ["]"], ["^"], ["`"], ["{"], ["|"], ["}"], ["~"], ["¡"], ["¢"], ["£"], ["¤"], ["¥"], ["§"], ["©"], ["ª"], ["«"], ["¬"], ["®"], ["°"], ["±"], ["º"], ["¹"], ["²"], ["³"], ["µ"], ["¶"], ["»"], ["¼"], ["½"], ["¾"], ["¿"], ["ƒ"], ["‘"], ["’"], ["‚"], ["‛"], ["“"], ["”"], ["„"], ["†"], ["‡"], ["•"], ["…"], ["‰"], ["‹"], ["›"], ["™"], ["Ω"], ["×"], ["÷"], ["Þ"], ["ß"], ["à"], ["á"], ["â"], ["ã"], ["ä"], ["å"], ["æ"], ["ç"], ["ć"], ["è"], ["é"], ["ê"], ["ë"], ["ę"], ["ē"], ["ì"], ["í"], ["î"], ["ï"], ["ł"], ["ð"], ["ñ"], ["ń"], ["ò"], ["ó"], ["ô"], ["õ"], ["ö"], ["ø"], ["ő"], ["œ"], ["š"], ["ù"], ["ú"], ["û"], ["ü"], ["ű"], ["ý"], ["ÿ"], ["ž"], ["ź"], ["ż"], ["þ"], ["À"], ["Á"], ["Â"], ["Ã"], ["Ä"], ["Å"], ["Æ"], ["Ç"], ["Ć"], ["È"], ["É"], ["Ê"], ["Ë"], ["Ę"], ["Ì"], ["Í"], ["Î"], ["Ï"], ["Ł"], ["Ð"], ["Ñ"], ["Ń"], ["Ò"], ["Ó"], ["Ô"], ["Õ"], ["Ö"], ["Ø"], ["Ő"], ["Œ"], ["Š"], ["Ù"], ["Ú"], ["Û"], ["Ü"], ["Ű"], ["Ý"], ["Ÿ"], ["Ž"], ["Ź"], ["Ż"] ],
	],
	
	published: {
		terminal: null
	},

	components: [
		{kind: "VirtualList", className: 'charSelectList', onSetupRow: "setupRow", components: [
			{kind: "Item", className: 'charSelectRow', name: 'charRow', layoutKind: 'HFlexLayout', components: [
				{kind: "charKey", name: 'b1', onmouseup: 'doMouseup'},
				{kind: "charKey", name: 'b2', onmouseup: 'doMouseup'},
				{kind: "charKey", name: 'b3', onmouseup: 'doMouseup'},
				{kind: "charKey", name: 'b4', onmouseup: 'doMouseup'},
				{kind: "charKey", name: 'b5', onmouseup: 'doMouseup'},
			]}
		]}
	],
	
	doMouseup: function(inSender, inEvent) {
		this.terminal.keyDown(0, inSender.sym, inSender.unicode, 1);
		this.terminal.keyUp(0, inSender.sym, inSender.unicode, 1);
		this.close()
	},
	
	create: function() {
		this.inherited(arguments);
		if (enyo.g11n.currentLocale().getLanguage() == 'en')
			this.data = this.chars[0].concat(this.chars[1])
		else
			this.data = this.chars[0].concat(this.chars[2])
	},
	
	setupRow: function(inSender, inIndex) {
		this.$.charRow.domStyles["border-top"] = "none"
		this.$.charRow.domStyles["border-bottom"] = "none"
		var max = Math.ceil(this.data.length/5)
		if (inIndex == max) return true
		if (inIndex >= 0 && inIndex < max) {
			var borderTop = "none"
			var borderBottom = "solid"
			if (inIndex == max-1)
				borderBottom = "none"
			this.$.b1.setBorder(borderTop, borderBottom, "none", "solid")
			this.$.b2.setBorder(borderTop, borderBottom, "none", "solid")
			this.$.b3.setBorder(borderTop, borderBottom, "none", "solid")
			this.$.b4.setBorder(borderTop, borderBottom, "none", "solid")
			this.$.b5.setBorder(borderTop, borderBottom, "none", "none")
			var i = inIndex * 5
			if (i+0 < this.data.length)
				this.$.b1.setData(this.data[i+0])
			else
				this.$.b1.setShowing(false)
			if (i+1 < this.data.length)
				this.$.b2.setData(this.data[i+1])
			else
				this.$.b2.setShowing(false)
			if (i+2 < this.data.length)
				this.$.b3.setData(this.data[i+2])
			else
				this.$.b3.setShowing(false)
			if (i+3 < this.data.length)
				this.$.b4.setData(this.data[i+3])
			else
				this.$.b4.setShowing(false)
			if (i+4 < this.data.length)
				this.$.b5.setData(this.data[i+4])
			else
				this.$.b5.setShowing(false)
			return true;
		}
	}
	
})

enyo.kind({
	
	name: 'charKey',
	kind: "CustomButton",
	width: '50px',
	height: '50px',
	layoutKind: 'HFlexLayout',
	pack: 'center',
	align: 'center',
	style: 'border-width: 1px; border-color: gray;',
	
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
	}

})

enyo.kind({
	
	name: 'CharSelector',
	kind: "Popup",
	
	className: 'enyo-popup charSelect',
	
	width: '300px',
	height: '300px',
	
	data: [ "$", "€", ";", "<", ">", "[", "\\", "]", "^", "`", "{", "|", "}", "~", "¡", "¢", "£", "¤", "¥", "§", "©", "ª", "«", "¬", "®", "°", "±", "º", "¹", "²", "³", "µ", "¶", "»", "¼", "½", "¾", "¿", "ƒ", "‘", "’", "‚", "‛", "“", "”", "„", "†", "‡", "•", "…", "‰", "‹", "›", "™", "Ω", "×", "÷", "Þ", "ß", "à", "á", "â", "ã", "ä", "å", "æ", "ç", "ć", "è", "é", "ê", "ë", "ę", "ē", "ì", "í", "î", "ï", "ł", "ð", "ñ", "ń", "ò", "ó", "ô", "õ", "ö", "ø", "ő", "œ", "š", "ù", "ú", "û", "ü", "ű", "ý", "ÿ", "ž", "ź", "ż", "þ", "À", "Á", "Â", "Ã", "Ä", "Å", "Æ", "Ç", "Ć", "È", "É", "Ê", "Ë", "Ę", "Ì", "Í", "Î", "Ï", "Ł", "Ð", "Ñ", "Ń", "Ò", "Ó", "Ô", "Õ", "Ö", "Ø", "Ő", "Œ", "Š", "Ù", "Ú", "Û", "Ü", "Ű", "Ý", "Ÿ", "Ž", "Ź", "Ż", ":-(", ":-)", ";-)"],
	
	published: {
		terminal: null
	},

	components: [
  	{kind: "VirtualList", width: '300px', height: '300px', style: 'padding: 0px; margin: 0px', onSetupRow: "setupRow", components: [
    	{kind: "Item", width: '300px', style: 'padding: 0px; margin: 0px', name: 'charRow', layoutKind: 'HFlexLayout', components: [
      	{kind: "charKey", name: 'b1', onmouseup: 'doMouseup'},
        {kind: "charKey", name: 'b2', onmouseup: 'doMouseup'},
        {kind: "charKey", name: 'b3', onmouseup: 'doMouseup'},
        {kind: "charKey", name: 'b4', onmouseup: 'doMouseup'},
        {kind: "charKey", name: 'b5', onmouseup: 'doMouseup'},
			]}
		]}
	],
	
	doMouseup: function(inSender, inEvent) {
		//this.log(inSender)
		this.terminal.inject(inSender.getCaption(), 1)
		this.close()
	},
	
	create: function() {
		this.inherited(arguments);
		//this.log(this.className)
	},
	
	setupRow: function(inSender, inIndex) {
		this.$.charRow.domStyles["border-top"] = "none"
		this.$.charRow.domStyles["border-bottom"] = "none"
		var max = Math.ceil(this.data.length/5)
		if (inIndex == max) return true
		if (inIndex >= 0 && inIndex < max) {
			var borderTop = "none"
			if (inIndex==0)
				borderTop = "solid"
			this.$.b1.setBorder(borderTop, "solid", "none", "solid")
			this.$.b2.setBorder(borderTop, "solid", "none", "solid")
			this.$.b3.setBorder(borderTop, "solid", "none", "solid")
			this.$.b4.setBorder(borderTop, "solid", "none", "solid")
			this.$.b5.setBorder(borderTop, "solid", "none", "none")
			var i = inIndex * 5
			if (i+0 < this.data.length)
				this.$.b1.setCaption(this.data[i+0])
			else
				this.$.b1.setShowing(false)
			if (i+1 < this.data.length)
				this.$.b2.setCaption(this.data[i+1])
			else
				this.$.b2.setShowing(false)
			if (i+2 < this.data.length)
				this.$.b3.setCaption(this.data[i+2])
			else
				this.$.b3.setShowing(false)
			if (i+3 < this.data.length)
				this.$.b4.setCaption(this.data[i+3])
			else
				this.$.b4.setShowing(false)
			if (i+4 < this.data.length)
				this.$.b5.setCaption(this.data[i+4])
			else
				this.$.b5.setShowing(false)
			return true;
		}
	}
	
})

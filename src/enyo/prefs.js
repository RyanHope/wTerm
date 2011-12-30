enyo.kind({
			
	kind: 'Popup2',
	name: 'Preferences',
	scrim: true,
	modal: true,
	autoClose: false,
	dismissWithClick: false,
	width: "420px",
	align: 'center',
		
	published: {
		prefs: null,
		hasBitstream: false,
		hasDejaVu: false,
		hasLiberation: false,
		hasDroid: false
	},
	
	fontsLoaded: false,
	
	events: {
		onClose: ''
	},
	
	components: [
		{
			layoutKind: "HFlexLayout",
			pack: "center",
			style: 'padding-bottom: 20px;',
			components: [
				{kind: "RadioGroup", name: "myGroup", onclick: "myGroupClick", value: 'general',
		      		components: [
		          		{caption: "General", value: "general", width: '120px'},
					]}
			]
		},
		{
			layoutKind: "HFlexLayout",
			pack: "center",
			className: "enyo-modaldialog-container",
			style: 'padding: 8px;',
			name: 'general',
			components: [
				{kind: "Group", flex: 1, components: [
					{kind: 'HFlexBox', align: "center", components: [
						{content: "Font Size", style: "padding-left: 8px", flex: 1},
						{kind: "IntegerPicker", name: 'fontSize', label: '', min: 8, max: 22}
					]}
  			]}
			]},
			{flex:1},
  		{
  			layoutKind: "HFlexLayout",
  			pack: "center",
  			components: [
  				{kind: 'Spacer'},
      			{
      				kind: "Button",
      				caption: "Close",
      				flex: 1,
      				onclick: "closePrefs"
  				},
  				{kind: 'Spacer'}
  			]
		}
	],
	
	myGroupClick: function(inSender, inEvent) {
		var grp = inSender.getValue()
		if (grp == 'general') {
			this.$.general.setShowing(true)
		}
	},
	
	rendered: function() {
		this.$.myGroup.setValue('general')
		this.$.fontSize.setValue(this.prefs.get('fontSize'))
	},
	
	closePrefs: function(inSender, inEvent) {
		this.prefs.set('fontSize',this.$.fontSize.value)
		this.doClose()
		this.close()
	},

})
enyo.kind({

	name: "wTermFirstUse",
	kind: enyo.VFlexBox,
			
	components: [
		{kind: 'Setup', name: 'setup', onPluginReady: 'pluginReady'},
		{name: "appManager", kind: "PalmService", service: enyo.palmServices.application},
		{kind:"PageHeader", className:"preferences-header", pack:"center", components: [
			{kind: "Image", src: "images/icon-setup-48.png", className: "preferences-header-image"},
			{kind: "Control", content: "Setup"}
		]},
 		{kind: "Scroller", flex: 1, components:[
 			{kind: "Control", className: "enyo-preferences-box", components: [
				{kind: "RowGroup", caption: "Root User", components: [
					{kind: "HFlexBox", align: "center", components: [
						{name: 'passStatus', content: "Password Status&nbsp;&nbsp;&nbsp;(?)", flex: 1, allowHtml: true},
						{kind: "Image", name: 'imgYes', src: "images/button_yes.png", showing: false},
						{kind: "Image", name: 'imgNo', src: "images/button_no.png", showing: false}
			      	]},
			      	{kind: "PasswordInput", hint: "New password", name: 'pass1', changeOnInput: true, onchange: 'verifyPassword'},
				  	{kind: "PasswordInput", hint: "Retype password", name: 'pass2', changeOnInput: true, onchange: 'verifyPassword'},
				  	{kind: "Button", content: "Set Password", className: 'enyo-button-affirmative', name: 'setPass', disabled: true, onclick: 'rootpassSet'}
			  	]},
				{
					content: "In order to use the 'su' command and safely become root<br> you must set a root password.",
					style: 'font-size: 75%; text-align: center;',
					allowHtml: true
				},
				{style: 'height: 29px'},
				{kind: "RowGroup", caption: "Non-Root User (wTerm)", components: [
			      	{kind: "Input", value: "/bin/sh", disabled: true, components: [{className: 'enyo-label', content: 'Shell'}]}
			  	]},
				{style: 'height: 58px'},
				{kind: 'Button', content: 'Create Root Console Launchpoint', className: 'enyo-button-negative', onclick: "addRootConsoleToLauncher"},
			]}
 		]},
 		{kind: "Toolbar", pack: "center", className: "enyo-toolbar-light", components: [
            {kind: "Button", caption: "Done", onclick: "doClose", className: "enyo-preference-button enyo-button-dark"}
        ]},
	],
		
	verifyPassword: function() {
		if ((this.$.pass1.getValue() == this.$.pass2.getValue()) && this.$.pass1.getValue().length > 0)
			this.$.setPass.setDisabled(false)
		else
			this.$.setPass.setDisabled(true)
	},
	
	addRootConsoleToLauncher: function() {
		var rootLP = PREFS.get('rootLaunchPoint')
		if (rootLP)
			this.$.appManager.call(
				{launchPointId: rootLP},
				{method: "addLaunchPoint", onResponse: 'launchPointResponse'}
			);
		this.$.appManager.call(
			{id: enyo.fetchAppId(), icon: "images/icon-root-64.png", title: "wTerm (root)", params: {root: true}},
			{method: "addLaunchPoint", onResponse: 'launchPointResponse'}
		);
	},
	
	rootLaunchPointResponse: function(inSender, inResponse) {
		PREFS.set('rootLaunchPoint', inResponse.launchPointId)
	},
	
	setupLaunchPointResponse: function(inSender, inResponse) {
		PREFS.set('setupLaunchPoint', inResponse.launchPointId)
	},
	
	doClose: function() {
		this.$.setup.setupNonRoot()
		if (enyo.windowParams.setup)
			window.close()
		else {
			PREFS.set('firstUse', true)
			if (!PREFS.get('setupLaunchPoint')) {
				this.$.appManager.call(
					{id: enyo.fetchAppId(), icon: "images/icon-setup-64.png", title: "wTerm Setup", params: {setup: true}},
					{method: "addLaunchPoint", onResponse: 'rootLaunchPointResponse'}
				)
			}
			this.destroyComponents()
			this.createComponent({kind: "wTermApp"})
			this.render()
		}
	},
	
	rootpassSet: function() {
		this.$.setup.setPassword("root", this.$.pass1.getValue())
		if (this.$.setup.hasPassword("root"))
		{
			this.$.setup.addToGroup("wterm", "root")
			this.$.setup.setupSU(true)
		}
		this.$.pass1.setValue('')
		this.$.pass2.setValue('')
		this.$.setPass.setDisabled(true)
		this.setup()
	},
	
	setup: function() {
		if (this.$.setup.hasPassword("root"))
		{
			this.$.imgYes.setShowing(true)
			this.$.imgNo.setShowing(false)
			this.$.passStatus.setContent("Password Status&nbsp;&nbsp;&nbsp;(Set)")
		}
		else
		{
			this.$.imgYes.setShowing(false)
			this.$.imgNo.setShowing(true)
			this.$.passStatus.setContent("Password Status&nbsp;&nbsp;&nbsp;(Unset)")
		}
	},
	
	pluginReady: function() {
		this.log('Setup Plugin Ready')
		this.setup()
	}

})

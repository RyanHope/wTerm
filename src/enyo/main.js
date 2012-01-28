enyo.kind({

	name: 'wTermLauncher',
	kind: enyo.Component,
	
	components: [
		{name: "appManager", kind: "PalmService", service: enyo.palmServices.application}
	],
	
	initComponents: function() {
  		this.inherited(arguments)
		this.createComponent({kind: 'ApplicationEvents', onApplicationRelaunch: 'onRelaunch'})
	},
		
	create: function() {
		this.inherited(arguments);
		enyo.application.m = this;
    },

	launch: function(relaunch) {
		if (enyo.windowParams.resetFirstUse)
			PREFS.set('firstUse', false)
		else if (enyo.windowParams.removeLaunchPoints)
			this.removeLaunchPoints()
		else if (enyo.windowParams.dockMode || enyo.windowParams.windowType === 'dockModeWindow')
			enyo.windows.openWindow('dock.html', 'dock', enyo.windowParams, {window:"dockMode"});
		else
			enyo.windows.openWindow('app.html', null, enyo.windowParams, null);
	},
	
	onRelaunch: function() {
		this.launch(true)
	},
	
	removeLaunchPoints: function() {
		this.$.appManager.call({launchPointId: PREFS.get('rootLaunchPoint')}, {method: "removeLaunchPoint"})
		this.$.appManager.call({launchPointId: PREFS.get('setupLaunchPoint')}, {method: "removeLaunchPoint"})
	}
	
})

enyo.kind({
	name: "Pullout",
	kind: enyo.Toaster,
	events: {
		onBypassClose: ""
	},
	lazy: false,
	components: [
		{name: "shadow", className: "enyo-sliding-view-shadow"},
		{kind: "VFlexBox", height: "100%", components: [
			{name: "client", flex: 1, layoutKind: "VFlexLayout"},
			{kind: "Toolbar", className: 'enyo-toolbar-light', align: "center", showing: false, components: [
				{name: "dragHandle", kind: "GrabButton", onclick: "close"}
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
	}
})
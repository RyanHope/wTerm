enyo.kind({
	name: "wi.Group",
	kind: enyo.Group,
	
	chrome: [
		{name: "llabel", kind: "Control", className: "enyo-group-label"},
		{name: "rlabel", kind: "Control", className: "enyo-group-label-right"},
		{name: "client", kind: "OrderedContainer", className: "enyo-group-inner"}
	],
	
	captionChanged: function() {
		if (this.caption instanceof Array) {
			this.$.llabel.setContent(this.caption[0]);
			this.$.llabel.setShowing(this.caption[0]);
			this.$.rlabel.setContent(this.caption[1]);
			this.$.rlabel.setShowing(this.caption[1]);
		} else {
			this.$.llabel.setContent(this.caption);
			this.$.rlabel.setShowing(false);
		}
		this.addRemoveClass("labeled", this.caption);
	},
	
	contentFitChanged: function() {
		if (this.contentFit) {
			this.createLayoutFromKind("VFlexLayout");
		} else {
			this.destroyObject("layout");
		}
		this.$.llabel.addRemoveClass("enyo-group-fit", this.contentFit);
		this.$.rlabel.addRemoveClass("enyo-group-fit", this.contentFit);
	},
	
})
enyo.kind({

	name: "wTermLauncher",
	kind: enyo.Component,
	
	statics: {
        FUNCTIONS_TO_RUN: [
			"_createPreferences",
			"_setupAppConstants",
   			"_setupCreateNewWindow",
   			"_setupGetFullWindowNameWithPrefix",
   			"_setupActivateOrCreateWindow",
   			"_setupRelaunch",
   			"_launchApp"]
	},
	
	create: function () {
        this.inherited(arguments);
        this.alreadyStarted = false;
        this.currentRunningFuncIndex = 0;
    },

	run: function () {
    	if (this.alreadyStarted) {
            throw new Error("Startup.run has already been called!");
        }
        this.alreadyStarted = true;
        this._runNextStartupFunc();
	},
	
	_runNextStartupFunc: function () {
		var nextFuncName = wTermLauncher.FUNCTIONS_TO_RUN[this.currentRunningFuncIndex];
		if (nextFuncName) {
			var nextFunc = this[nextFuncName];
			if (nextFunc && enyo.isFunction(nextFunc)) {
				nextFunc.call(this, this._runNextStartupFunc.bind(this));
				this.currentRunningFuncIndex++;
			} else {
				throw new Error("Startup method not found!  name: " + stringify(nextFuncName));
			}
		}
	},
	
	_createPreferences: function (finishedCallback) {
		if (!enyo.application.prefs) {
            enyo.application.prefs = new Prefs();
        }
        Utils.defer(finishedCallback);
	},
	
	_setupAppConstants: function (finishedCallback) {
        enyo.application.WTERM_CONSTANTS = {
            BASE_APPLICATION_WINDOW_NAME: "app"
        };
        Utils.defer(finishedCallback);
    },

	_setupCreateNewWindow: function (finishedCallback) {
        enyo.application.createNewWindow = (function () {                                                                                                                                            
            var windowCounter = 0;
            return function (windowName, windowParams, windowAttributes, windowInfo) {                                               
                windowName = windowName || enyo.application.WTERM_CONSTANTS.BASE_APPLICATION_WINDOW_NAME;                                                                                                                                                                  
                windowCounter++;                                                                                                            
                var newWindowName = windowName + "-" + windowCounter;
				enyo.windows.activate(windowName + "Index.html", newWindowName, windowParams, windowAttributes, windowInfo);
				return newWindowName;
			};                                                                                                                           
		})();
        Utils.defer(finishedCallback);
    },
    
    _setupGetFullWindowNameWithPrefix: function (finishedCallback) {
        enyo.application.getFullWindowNameWithPrefix = function (windowNamePrefix) {
            var existingWindows = enyo.windows.getWindows();
            var existingWindowNames = Object.keys(existingWindows);
            var existingWindowName = existingWindowNames.detect(function (existingName) {
                return (existingName.indexOf(windowNamePrefix) === 0);
            });
            return existingWindowName;
        };
        Utils.defer(finishedCallback);
    },

    _setupActivateOrCreateWindow: function (finishedCallback) {
        enyo.application.activateOrCreateWindow = function (windowName, windowParams, windowAttributes, windowInfo) {
            var existingWindowName = enyo.application.getFullWindowNameWithPrefix(windowName);
            if (existingWindowName) {
                enyo.windows.activate(undefined, existingWindowName, windowParams);
                return existingWindowName;
            } else {
                return enyo.application.createNewWindow(windowName, windowParams, windowAttributes, windowInfo);
            }
        };
        Utils.defer(finishedCallback);
    },

    _setupRelaunch: function (finishedCallback) {
        enyo.relaunch = function (inParams) {
            var activeWindow = (Utils.onDevice()) ? (enyo.windows && enyo.windows.getActiveWindow()) || window : window;
            if (!inParams || Object.keys(inParams).length === 0) {
                inParams = (activeWindow.PalmSystem && activeWindow.PalmSystem.launchParams && JSON.parse(activeWindow.PalmSystem.launchParams)) || {};
            }
            var windowParams = inParams || {};
		    var windowName = windowParams.stage || enyo.application.WTERM_CONSTANTS.BASE_APPLICATION_WINDOW_NAME;
            var windowAttributes = {};
            if (inParams.dockMode || inParams.windowType === "dockModeWindow") {
                windowName = "dock";
                windowAttributes.window = "dockMode";
            }
            if (!inParams.newWindow) {
                enyo.application.activateOrCreateWindow(windowName, windowParams, windowAttributes)
            } else {
                enyo.application.createNewWindow(windowName, windowParams, windowAttributes);
            }
        };
        enyo.applicationRelaunchHandler = enyo.relaunch;
        Utils.defer(finishedCallback);
    },

    _launchApp: function (finishedCallback) {
        enyo.relaunch();
        Utils.defer(finishedCallback);
    }
	
})

/*
 * Copyright (C) 2018 LEIDOS.
 *
 * Licensed under the Apache License, Version 2.0 (the "License"); you may not
 * use this file except in compliance with the License. You may obtain a copy of
 * the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the
 * License for the specific language governing permissions and limitations under
 * the License.
 */

package gov.dot.fhwa.saxton.carma.guidance.plugins;

import gov.dot.fhwa.saxton.carma.guidance.plugins.PluginLifecycleHandler.PluginState;
import gov.dot.fhwa.saxton.carma.guidance.util.ILogger;
import gov.dot.fhwa.saxton.carma.guidance.util.LoggerManager;
import java.util.HashMap;
import java.util.Map;

/**
 * Class responsible for coordinating the execution of multiple plugins
 * <p>
 * Creates {@link PluginLifecycleHandler} instances for each plugin then provides access to them
 * based on the plugin name and plugin version values.
 */
public class PluginExecutor {

    protected Map<String, PluginLifecycleHandler> lifecycleHandlers = new HashMap<>();
    protected ILogger log = LoggerManager.getLogger();
    protected PluginServiceLocator pluginServiceLocator;

    PluginExecutor() {
    }

    /**
     * Submit a plugin for management by the PluginExecutor
     * <p>
     * This plugin will be indexed by it's name and versionId fields, which are assumed to be unique.
     * In the event these are not unique, a duplicate submission will result in the older submission
     * being replaced.
     * <p>
     * Note that this method does not begin the execution, it simply sets up the threading framework
     * to support that. To begin the exection call {@link PluginExecutor#initializePlugin(String, String)}
     * and {@link PluginExecutor#resumePlugin(String, String)}.
     *
     * @param plugin The plugin instance to begin tracking.
     */
    public void submitPlugin(IPlugin plugin) {
        PluginLifecycleHandler handler = new PluginLifecycleHandler(plugin);
        lifecycleHandlers.put(plugin.getVersionInfo().componentName() + plugin.getVersionInfo().revisionString(), handler);
    }

    /**
     * Asynchronously invoke a plugin's {@link IPlugin#onInitialize()} method
     * <p>
     * Locates the {@link PluginLifecycleHandler} instance associated with the specified plugin
     * and causes it to call the plugins initialization behavior.
     *
     * @param pluginName    The string plugin name as reported by {@link IPlugin#getName()}
     * @param pluginVersion The string plugin version as reported by {@link IPlugin#getVersionId()}
     */
    public void initializePlugin(String pluginName, String pluginVersion) {
        lifecycleHandlers.get(pluginName + pluginVersion).initialize();
    }

    /**
     * Asynchronously invoke a plugin's {@link IPlugin#onResume()} ()} method, then indefinitely
     * loop over that plugin's {@link IPlugin#loop()} method.
     * <p>
     * Locates the {@link PluginLifecycleHandler} instance associated with the specified plugin
     * and causes it to call the plugins resume and loop behavior.
     *
     * @param pluginName    The string plugin name as reported by {@link IPlugin#getName()}
     * @param pluginVersion The string plugin version as reported by {@link IPlugin#getVersionId()}
     */
    public void resumePlugin(String pluginName, String pluginVersion) {
        lifecycleHandlers.get(pluginName + pluginVersion).resume();
    }

    /**
     * Asynchronously invoke a plugin's {@link IPlugin#onSuspend()} ()} method
     * <p>
     * Locates the {@link PluginLifecycleHandler} instance associated with the specified plugin
     * and causes it to call the plugins suspend behavior
     *
     * @param pluginName    The string plugin name as reported by {@link IPlugin#getName()}
     * @param pluginVersion The string plugin version as reported by {@link IPlugin#getVersionId()}
     */
    public void suspendPlugin(String pluginName, String pluginVersion) {
        lifecycleHandlers.get(pluginName + pluginVersion).suspend();
    }

    /**
     * Asynchronously invoke a plugin's {@link IPlugin#onTerminate()} method
     * <p>
     * Locates the {@link PluginLifecycleHandler} instance associated with the specified plugin
     * and causes it to call the plugins terminate behavior. Then deletes the associated
     * PluginLifeCycleHandler
     *
     * @param pluginName    The string plugin name as reported by {@link IPlugin#getName()}
     * @param pluginVersion The string plugin version as reported by {@link IPlugin#getVersionId()}
     */
    public void terminatePlugin(String pluginName, String pluginVersion) {
        lifecycleHandlers.get(pluginName + pluginVersion).terminate();
        lifecycleHandlers.remove(pluginName + pluginVersion);
    }

    /**
     * Get the current state of the specified plugin from its corresponding handler.
     * 
     * If the plugin is not initialized for any reason UNINITIALIZED will be returned
     * 
     * @param pluginName    The string plugin name as reported by {@link IPlugin#getName()}
     * @param pluginVersion The string plugin version as reported by {@link IPlugin#getVersionId()}
     * @return The plugin state as a PluginState value
     */
    public PluginState getPluginState(String pluginName, String pluginVersion) {
        PluginLifecycleHandler handler = null;
        handler = lifecycleHandlers.get(pluginName + pluginVersion);
        return (handler != null ? handler.getState() : PluginState.UNINITIALIZED);
    }
}

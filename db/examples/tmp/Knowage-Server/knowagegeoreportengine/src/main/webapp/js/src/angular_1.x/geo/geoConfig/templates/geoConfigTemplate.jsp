
<expander-box id="legend" expanded='true'  title='{{::translate.load("gisengine.info.message.legend.config")}}'> 
	<md-tabs md-dynamic-height="" >
      <md-tab label='{{translate.load("gisengine.rigthMapMenu.analysisType.choropleth")}}'>
        <md-content>
          <md-input-container class="md-block">
			   <label>{{translate.load("gisengine.info.message.legend.config.method")}}</label>
			    <md-select ng-model="template.analysisConf.choropleth.method">
			      <md-option ng-repeat="meth in choroplethMethodTypeList" value="{{meth.value}}">
			        {{meth.label}}
			      </md-option>
			    </md-select>
			</md-input-container>
        
			<md-input-container class="md-block">
				<label>{{translate.load("gisengine.info.message.legend.config.nOfClasses")}}</label>
				 <md-select ng-model="template.analysisConf.choropleth.classes">
				   <md-option ng-repeat="n in [2,3,4,5,6,7]" value="{{n}}">
				     {{n}}
				   </md-option>
				 </md-select>
		    </md-input-container>
		    
		    <md-input-container class="md-block colorInputContainer">
				<label>{{translate.load("gisengine.info.message.legend.config.fromColor")}}</label>
				  <color-picker color-picker-swatch="true"  color-picker-format="'rgb'" ng-model="template.analysisConf.choropleth.fromColor"></color-picker>
		    </md-input-container>
		    
		     <md-input-container class="md-block colorInputContainer">
				<label>{{translate.load("gisengine.info.message.legend.config.toColor")}}</label>
				  <color-picker  color-picker-swatch="true" color-picker-format="'rgb'" ng-model="template.analysisConf.choropleth.toColor"></color-picker>
		    </md-input-container>
 
		</md-content>
      </md-tab>
      <md-tab label='{{translate.load("gisengine.rigthMapMenu.analysisType.proportionalSymbol")}}'>
        <md-content>
         <md-input-container class="md-block colorInputContainer">
				<label>{{translate.load("gisengine.info.message.legend.config.color")}}</label>
				  <color-picker  color-picker-swatch="true" color-picker-format="'rgb'" ng-model="template.analysisConf.proportionalSymbol.color"></color-picker>
		    </md-input-container>
<!--           <div md-color-picker value="template.analysisConf.proportionalSymbol.color"  label='{{translate.load("gisengine.info.message.legend.config.color")}}' open-on-input="true" style="width: 100%;" ></div> -->
		    <span >{{translate.load("gisengine.info.message.legend.config.value.min")}}</span>
	      	<md-slider flex class="visibleValue md-primary margintop " md-discrete="" ng-model="template.analysisConf.proportionalSymbol.minRadiusSize" step="1" min="0" max="{{template.analysisConf.proportionalSymbol.maxRadiusSize}}" aria-label="rating"></md-slider>
			<span >{{translate.load("gisengine.info.message.legend.config.value.max")}}</span>
	      	<md-slider flex class="md-primary visibleValue margintop" md-discrete="" ng-model="template.analysisConf.proportionalSymbol.maxRadiusSize" step="1" min="{{template.analysisConf.proportionalSymbol.minRadiusSize}}" max="50" aria-label="rating"></md-slider>
		</md-content>
      </md-tab>
      <md-tab label='{{translate.load("gisengine.rigthMapMenu.analysisType.chart")}}'>
        <md-content>
          
         <md-input-container ng-repeat="ind in indicators" class="md-block colorInputContainer" ng-init="template.analysisConf.chart['indicator_'+($index+1)] = template.analysisConf.chart['indicator_'+($index+1)] || 'green'">
				<label>{{translate.load("gisengine.info.message.legend.config.color")}}</label>
				  <color-picker  color-picker-swatch="true" color-picker-format="'rgb'" ng-model="template.analysisConf.chart['indicator_'+($index+1)]"></color-picker>
		    </md-input-container>
		    
          
        </md-content>
      </md-tab>
      <md-tab label='{{translate.load("gisengine.rigthMapMenu.analysisType.accessibilityRanges")}}'>
        <md-content>
        <md-chips class="smallChips" ng-model="template.accessibilityConf.ranges" readonly="false" md-removable="true">
	      <md-chip-template>
	        <span>
	          <strong>[{{template.accessibilityConf.ranges.indexOf($chip)}}] {{$chip}}</strong>
	        </span>
	      </md-chip-template>
	      <md-button md-chip-remove class="md-icon-button">
	        <md-icon md-font-icon="fa fa-times-circle"></md-icon>
	      </md-button>
	      <input type="text" placeholder="Add range name">
	    </md-chips>
        </md-content>
       </md-tab>
    </md-tabs>
			
		

</expander-box>




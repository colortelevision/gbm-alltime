const id = "EVENT_GRIDDUEL_NATIVE";
const fields = [
 {key:"action",label:"Native action",type:"select",options:[["init","Initialize duel"],["tick","Update / render (Actor On Update)"],["fixture","TEST only: scenario in variable"]],defaultValue:"tick"},
 {key:"variable",label:"Exit status / TEST scenario",type:"variable",defaultValue:"240"},
 {label:"Finite banked callbacks. 0=playing, 1=win, 2=lose, 3=quit. Data: plugins/Gridduel/data/catalog.json (regenerate catalog after edits)."}
];
const compile=(input,helpers)=>{
 const fn=input.action==="init"?"gd_init":input.action==="fixture"?"gd_test_fixture":"gd_tick";
 helpers._invoke(fn,0,helpers.getVariableAlias(input.variable));
};
module.exports={id,name:"Gridduel: Native Step",groups:["EVENT_GROUP_MISC"],fields,compile};

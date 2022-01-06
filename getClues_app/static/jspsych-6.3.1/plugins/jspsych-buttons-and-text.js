/**
 * jspsych-buttons-and-text
 * a jspsych plugin for free response survey questions
 *
 * Josh de Leeuw
 *
 * documentation: docs.jspsych.org
 *
 */


jsPsych.plugins['buttons-and-text'] = (function() {

    var plugin = {};
  
    plugin.info = {
      name: 'buttons-and-text',
      description: '',
      parameters: {
        questions: {
          type: jsPsych.plugins.parameterType.COMPLEX,
          array: true,
          pretty_name: 'Questions',
          default: undefined,
          nested: {
            prompt: {
              type: jsPsych.plugins.parameterType.STRING,
              pretty_name: 'Prompt',
              default: undefined,
              description: 'Prompt for the subject to response'
            },
            placeholder: {
              type: jsPsych.plugins.parameterType.STRING,
              pretty_name: 'Placeholder',
              default: "",
              description: 'Placeholder text in the textfield.'
            },
            rows: {
              type: jsPsych.plugins.parameterType.INT,
              pretty_name: 'Rows',
              default: 1,
              description: 'The number of rows for the response text box.'
            },
            columns: {
              type: jsPsych.plugins.parameterType.INT,
              pretty_name: 'Columns',
              default: 40,
              description: 'The number of columns for the response text box.'
            },
            required: {
              type: jsPsych.plugins.parameterType.BOOL,
              pretty_name: 'Required',
              default: false,
              description: 'Require a response'
            },
            name: {
              type: jsPsych.plugins.parameterType.STRING,
              pretty_name: 'Question Name',
              default: '',
              description: 'Controls the name of data values associated with this question'
            }
          }
        },
        preamble: {
          type: jsPsych.plugins.parameterType.STRING,
          pretty_name: 'Preamble',
          default: null,
          description: 'HTML formatted string to display at the top of the page above all the questions.'
        },
        button_label: {
          type: jsPsych.plugins.parameterType.STRING,
          pretty_name: 'Button label',
          default:  'Continue',
          description: 'The text that appears on the button to finish the trial.'
        },
        choices: {
          type: jsPsych.plugins.parameterType.STRING,
          pretty_name: 'Choices',
          default: undefined,
          array: true,
          description: 'The labels for the buttons.'
        },
        button_html: {
          type: jsPsych.plugins.parameterType.STRING,
          pretty_name: 'Button HTML',
          default: '<button class="jspsych-btn">%choice%</button>',
          array: true,
          description: 'The html of the button. Can create own style.'
        },
        margin_vertical: {
          type: jsPsych.plugins.parameterType.STRING,
          pretty_name: 'Margin vertical',
          default: '0px',
          description: 'The vertical margin of the button.'
        },
        margin_horizontal: {
          type: jsPsych.plugins.parameterType.STRING,
          pretty_name: 'Margin horizontal',
          default: '8px',
          description: 'The horizontal margin of the button.'
        },
        autocomplete: {
          type: jsPsych.plugins.parameterType.BOOL,
          pretty_name: 'Allow autocomplete',
          default: false,
          description: "Setting this to true will enable browser auto-complete or auto-fill for the form."
        }
      }
    }
  
    plugin.trial = function(display_element, trial) {
  
      for (var i = 0; i < trial.questions.length; i++) {
        if (typeof trial.questions[i].rows == 'undefined') {
          trial.questions[i].rows = 1;
        }
      }
      for (var i = 0; i < trial.questions.length; i++) {
        if (typeof trial.questions[i].columns == 'undefined') {
          trial.questions[i].columns = 40;
        }
      }
      for (var i = 0; i < trial.questions.length; i++) {
        if (typeof trial.questions[i].value == 'undefined') {
          trial.questions[i].value = "";
        }
      }
  
      var html = '  <br>';
      // show preamble text
      if(trial.preamble !== null){
        html += '<div id="jspsych-buttons-and-text-preamble" class="jspsych-buttons-and-text-preamble">'+trial.preamble+'</div>';
      }
      
      // start form
      if (trial.autocomplete) {
        html += '<form id="jspsych-buttons-and-text-form">';
      } else {
        html += '<form id="jspsych-buttons-and-text-form" autocomplete="off">';
      }

      //display buttons
      var buttons = [];
      if (Array.isArray(trial.button_html)) {
          if (trial.button_html.length == trial.choices.length) {
              buttons = trial.button_html;
          } else {
              console.error('Error in buttons-and-text plugin. The length of the button_html array does not equal the length of the choices array');
          }
      } else {
          for (var i = 0; i < trial.choices.length; i++) {
              buttons.push(trial.button_html);
          }
      }
      html += '<div id="jspsych-buttons-and-text-btngroup">';
      for (var i = 0; i < trial.choices.length; i++) {
          if ((i % 4) ==0 ) {
              html += '<br>';
          }
          var str = buttons[i].replace(/%choice%/g, trial.choices[i]);
          html += '<div class="jspsych-buttons-and-text-button" style="display: inline-block; margin:' + trial.margin_vertical + ' ' + trial.margin_horizontal + '" id="jspsych-buttons-and-text-button-' + i + '" data-choice="' + i + '">' + str + '</div>';
      }

      // generate question order
      var question_order = [];
      for(var i=0; i<trial.questions.length; i++){
        question_order.push(i);
      }
      if(trial.randomize_question_order){
        question_order = jsPsych.randomization.shuffle(question_order);
      }
  
      // add questions
      for (var i = 0; i < trial.questions.length; i++) {
        var question = trial.questions[question_order[i]];
        var question_index = question_order[i];
        html += '<div id="jspsych-buttons-and-text-'+question_index+'" class="jspsych-buttons-and-text-question" style="margin: 2em 0em;">';
        html += '<p class="jspsych-buttons-and-text">' + question.prompt + '</p>';
        var autofocus = i == 0 ? "autofocus" : "";
        var req = question.required ? "required" : "";
        if(question.rows == 1){
          html += '<input type="text" id="input-'+question_index+'"  name="#jspsych-buttons-and-text-response-' + question_index + '" data-name="'+question.name+'" size="'+question.columns+'" '+autofocus+' '+req+' placeholder="'+question.placeholder+'"></input>';
        } else {
          html += '<textarea id="input-'+question_index+'" name="#jspsych-buttons-and-text-response-' + question_index + '" data-name="'+question.name+'" cols="' + question.columns + '" rows="' + question.rows + '" '+autofocus+' '+req+' placeholder="'+question.placeholder+'"></textarea>';
        }
        html += '</div>';
      }

      // add submit button
      html += '<input type="submit" id="jspsych-buttons-and-text-next" class="jspsych-btn jspsych-buttons-and-text" value="'+trial.button_label+'"></input>';
  
      html += '</form>'
      display_element.innerHTML = html;

      var start_time = performance.now();

      // backup in case autofocus doesn't work
      display_element.querySelector('#input-'+question_order[0]).focus();
  
      var choiceList = [];
      // add event listeners to buttons
      for (var i = 0; i < trial.choices.length; i++) {
        display_element.querySelector('#jspsych-buttons-and-text-button-' + i).addEventListener('click', function(e){
          //add to list when clicked, or if already in list, remove
          choiceIndex = e.currentTarget.getAttribute('data-choice');
          if(choiceList.includes(trial.choices[choiceIndex])) {
            choiceList.splice(choiceList.indexOf(trial.choices[choiceIndex]), 1);
            deactivateItem(e.target);
          } else {
            choiceList.push(trial.choices[choiceIndex]);
            activateItem(e.target);
          }
        });
      }

      function activateItem(e){
        e.className += "-activeItem";
      }

      function deactivateItem(e){
        e.className = e.className.substring(0, e.className.length - 11);
      }


      // store response
      /*
      var response = {
        rt: null,
        button: null
      };
      */

      //stop buttons from automatically submitting
      display_element.querySelector('#jspsych-buttons-and-text-form').onsubmit = function () { 
        return false
      };

      display_element.querySelector('#jspsych-buttons-and-text-next').addEventListener('click', function(e){
        //do not continue if less than 3 words selected or no clue given
        if ((choiceList.length != 3) || (document.querySelector('#jspsych-buttons-and-text-0').querySelector('textarea, input').value.length == 0)) {
          return false
        };
        e.preventDefault();
        // measure response time
        var endTime = performance.now();
        var response_time = endTime - startTime;
  
        // create object to hold responses
        var question_data = {};
        
        for(var index=0; index < trial.questions.length; index++){
          var id = "Q" + index;
          var q_element = document.querySelector('#jspsych-buttons-and-text-'+index).querySelector('textarea, input'); 
          var val = q_element.value;
          var name = q_element.attributes['data-name'].value;
          if(name == ''){
            name = id;
          }        
          var obje = {};
          obje[name] = val;
          Object.assign(question_data, obje);
        }
        // save data
        var trialdata = {
          rt: response_time,
          response: question_data,
          chosen_words: choiceList,
          choices: trial.choices
        };
  
        display_element.innerHTML = '';
  
        // next trial
        jsPsych.finishTrial(trialdata);
      });
  
      var startTime = performance.now();
    };
  
    return plugin;
  })();
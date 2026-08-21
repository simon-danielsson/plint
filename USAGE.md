## A walkthrough of Plint's HTML template engine.
  
### General
   
#### Reserved keywords
* include, for, in, if, endif, endfor

#### Regarding nesting
* Nested 'if' and 'for' statements are supported, with a maximum nesting 
  level of 4 (this number is arbitrary, and only there to counteract potential 
  overflow bugs in the recursion logic - can be adjusted at your own risk).

#### Variables
* Variables are added in code via the Plint_append_variable() function.

* Any variables you add are appended to a global scope, and can therefore
  be accessed from anywhere in your html documents (the drawback of this
  is that name collisions and shadowing can occur if you're not careful!)
  
---
  
### Variables
  
#### Example

``` html

   <div>
      <p>{{ my_var }}</p>
   </div>
```
   
---
   
### Keyword: include
  
* Include html content from a path resolved from the directory of program
  execution.
* Note that filepaths should not be surrounded by double/single quotes!
* A good rule-of-thumb to stay organized is to keep your included html files 
  in a separate "incl" directory.
  
#### Example
  
``` html
   <body>
      {% include incl/banner.html %}
      ...
   </body>
```
   
---
   
### Keyword: if, endif
  
#### Example 1
``` html
   {% if show_blog_section %}
       <div>My blog!</div>
   {% endif %}
```

#### Example 2 (+ negation operator)

``` html
   {% if !show_blog_section %}
      <div>No blog for you!</div>
   {% endif %}
```
  
---
   
### Keyword: for, endfor

#### Example 1 (accessing an index of an array, like any other variable)

``` html
   <div>
       <p>{{ my_array_var[7] }}</p>
   </div>
```

#### Example 2 (basic for-loop)

``` html
   <div class="footer-contact">
   <h3>Contact</h3>
      {% for dup_info in arr_contact_info %}
      <div>
         <p>{{ dup_info }}</p>
      </div
      {% endfor %}
   </div>
```

#### Example 3a (nested loops)

``` html
   <div class="testimonials">
   <h3>Testimonials</h3>
      {% for card in testimonials %}
      <div>
          {% for div in testimonial_div %}
            {{ div }}
          {% endfor %}
         {{ card }}
      </div>
      {% endfor %}
   </div>
```

#### Example 3b (nested loops)

``` html
   <!-- Pay attention to that the 'socials' array contains -->
   <!-- variable arrays within it that can be accessed by index. -->

   <!-- Theoretically you should be able to create a nested for-loop -->
   <!-- with an array of an array, but I have not bothered testing this -->
   <!-- since it's a stupid idea. -->

   <!-- Note that for use-cases like the following example, the iterated 
   <!-- array ought to only contain ARRAYS, not mix of different 
   <!-- variable kinds! -->

   <h3>Socials</h3>
   <div class="socials">
      {% for social in social_fields %}
      <span class="icon">{{ social[3] }}<\span>
      <a href="{{ social[0] }}">{{ social[1] }}</a>
      {% endfor %}
   </div>
```

